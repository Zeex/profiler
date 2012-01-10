// AMX profiler for SA-MP server: http://sa-mp.com
//
// Copyright (C) 2011 Sergey Zolotarev
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
	#include <Windows.h>
#endif

#include <boost/algorithm/string.hpp>

#include "samp_profiler/debug_info.h"
#include "samp_profiler/html_printer.h"
#include "samp_profiler/profiler.h"
#include "samp_profiler/text_printer.h"
#include "samp_profiler/xml_printer.h"

#include "amx_name.h"
#include "config_reader.h"
#include "jump-x86.h"
#include "plugin.h"
#include "version.h"

typedef void (*logprintf_t)(const char *format, ...);

extern void *pAMXFunctions;

static logprintf_t logprintf;

static std::list<AMX*> loaded_scripts;

static JumpX86 ExecHook;
static JumpX86 CallbackHook;

static int AMXAPI Exec(AMX *amx, cell *retval, int index) {
	ExecHook.Remove();
	CallbackHook.Install();

	int error = AMX_ERR_NONE;

	samp_profiler::Profiler *prof = samp_profiler::Profiler::Get(amx);
	if (prof != 0) {
		error =  prof->Exec(retval, index);
	} else {
		error = amx_Exec(amx, retval, index);
	}

	CallbackHook.Remove();
	ExecHook.Install();

	return error;
}

static int AMXAPI Callback(AMX *amx, cell index, cell *result, cell *params) {
	CallbackHook.Remove();
	ExecHook.Install();

	amx->sysreq_d = 0;

	int error = AMX_ERR_NONE;

	samp_profiler::Profiler *prof = samp_profiler::Profiler::Get(amx);
	if (prof != 0) {
		error =  prof->Callback(index, result, params);
	} else {
		error = amx_Callback(amx, index, result, params);
	}

	ExecHook.Remove();
	CallbackHook.Install();

	return error;
}

static std::string ToUnixPath(const std::string &path) {
	std::string fsPath = path;
	std::replace(fsPath.begin(), fsPath.end(), '\\', '/');
	return fsPath;
}

static bool IsGameMode(const std::string &amxName) {
	return ToUnixPath(amxName).find("gamemodes/") != std::string::npos;
}

static bool IsFilterScript(const std::string &amxName) {
	return ToUnixPath(amxName).find("filterscripts/") != std::string::npos;
}

static bool GetPublicVariable(AMX *amx, const char *name, cell &value) {
	cell  amx_addr;
	if (amx_FindPubVar(amx, name, &amx_addr) == AMX_ERR_NONE) {
		cell *phys_addr;
		amx_GetAddr(amx, amx_addr, &phys_addr);
		value = *phys_addr;
		return true;
	}
	return false;
}

static bool WantsProfiler(const std::string &amxName) {
	std::string goodAmxName = ToUnixPath(amxName);

	ConfigReader server_cfg("server.cfg");
	if (IsGameMode(amxName)) {
		if (server_cfg.GetOption("profile_gamemode", false)) {
			return true;
		}
	} else if (IsFilterScript(amxName)) {
		std::string fsList = server_cfg.GetOption("profile_filterscripts", std::string(""));
		std::stringstream fsStream(fsList);
		do {
			std::string fsName;
			fsStream >> fsName;
			if (goodAmxName == "filterscripts/" + fsName + ".amx"
					|| goodAmxName == "filterscripts/" + fsName) {
				return true;
			}
		} while (!fsStream.eof());
	}

	return false;
}

#ifdef _WIN32

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx);

static BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
	switch (dwCtrlType) {
	case CTRL_CLOSE_EVENT:
	case CTRL_BREAK_EVENT:
		for (std::list<AMX*>::const_iterator iterator = ::loaded_scripts.begin();
				iterator != ::loaded_scripts.end(); ++iterator) {
			AmxUnload(*iterator);
		}
	}
	return FALSE;
}

#endif

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() {
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

static void *AMXAPI my_amx_Align(void *v) { return v; }

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData) {
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];

	((void**)pAMXFunctions)[PLUGIN_AMX_EXPORT_Align16] = (void*)my_amx_Align;
	((void**)pAMXFunctions)[PLUGIN_AMX_EXPORT_Align32] = (void*)my_amx_Align;
	((void**)pAMXFunctions)[PLUGIN_AMX_EXPORT_Align64] = (void*)my_amx_Align;

	ExecHook.Install(
		((void**)pAMXFunctions)[PLUGIN_AMX_EXPORT_Exec],
		(void*)::Exec);
	CallbackHook.Install(
		((void**)pAMXFunctions)[PLUGIN_AMX_EXPORT_Callback],
		(void*)::Callback);

	#ifdef _WIN32
		SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
	#endif

	logprintf("  Profiler v"PROFILER_VERSION" is OK.");

	return true;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx) {
	::loaded_scripts.push_back(amx);

	std::string filename = GetAmxName(amx);
	if (filename.empty()) {
		logprintf("[profiler]: Can't find matching .amx file");
		return AMX_ERR_NONE;
	}

	if (!samp_profiler::Profiler::IsScriptProfilable(amx)) {
		logprintf("[profiler]: Can't profile '%s' (are you using -d0?)", filename.c_str());
		return AMX_ERR_NONE;
	}

	cell profiler_enabled = false;
	if (GetPublicVariable(amx, "profiler_enabled", profiler_enabled)
			&& !profiler_enabled) {
		return AMX_ERR_NONE;
	}

	if (profiler_enabled || WantsProfiler(filename)) {
		if (samp_profiler::DebugInfo::HasDebugInfo(amx)) {
			samp_profiler::DebugInfo debug_info;
			debug_info.Load(filename);
			if (debug_info.IsLoaded()) {
				logprintf("[profiler]: Loaded debug info from '%s'", filename.c_str());
				samp_profiler::Profiler::Attach(amx, debug_info);
				logprintf("[profiler]: Attached profiler to '%s'", filename.c_str());
				return AMX_ERR_NONE;
			} else {
				logprintf("[profiler]: Error loading debug info from '%s'", filename.c_str());
			}
		}
		samp_profiler::Profiler::Attach(amx);
		logprintf("[profiler]: Attached profiler to '%s' (no debug symbols)", filename.c_str());
	}

	return AMX_ERR_NONE;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx) {
	samp_profiler::Profiler *prof = samp_profiler::Profiler::Get(amx);

	if (prof != 0) {
		std::string amx_path = GetAmxName(amx);
		std::string amx_name = std::string(amx_path, 0, amx_path.find_last_of("."));

		ConfigReader server_cfg("server.cfg");

		std::string format =
			server_cfg.GetOption("profile_format", std::string("html"));
		boost::algorithm::to_lower(format);

		std::string filename = amx_name + "-profile";
		samp_profiler::Printer *printer = 0;

		if (format == "html") {
			filename += ".html";
			printer = new samp_profiler::HtmlPrinter;
		} else if (format == "text") {
			filename += ".txt";
			printer = new samp_profiler::TextPrinter;
		} else if (format == "xml") {
			filename += ".xml";
			printer = new samp_profiler::XmlPrinter;
		} else {
			logprintf("[profiler]: Unknown output format '%s'", format.c_str());
		}

		if (printer != 0) {
			std::ofstream ostream(filename.c_str());
			prof->PrintStats(amx_path, ostream, printer);
			delete printer;
		}

		samp_profiler::Profiler::Detach(amx);
	}

	return AMX_ERR_NONE;
}
