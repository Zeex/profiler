// SA:MP Profiler plugin
//
// Copyright (c) 2011 Zeex
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
//
// Basic configuration file reader. 
// Each line of an input file is a key-value pair: values begin after the first space, 
// so the keys cannot contain spaces. 
// Comments, multiple lines, quotes, escape sequences - these are not supported.

#include <map>
#include <sstream>
#include <string>

#ifndef SAMP_PROFILER_CONFIG_READER_H
#define SAMP_PROFILER_CONFIG_READER_H

namespace samp_profiler {

class ConfigReader {
public:
	ConfigReader();
	ConfigReader(const std::string &filename);

	bool LoadFile(const std::string &filename);

	template<typename T>
	T GetOption(const std::string &name, const T &defaultValue) const;

	bool IsLoaded() const { return loaded_; }

private:
	bool loaded_;
	std::map<std::string, std::string> options_;
};

template<typename T>
T ConfigReader::GetOption(const std::string &name, const T &defaultValue) const {
	std::map<std::string, std::string>::const_iterator it = options_.find(name);
	if (it == options_.end()) {
		return defaultValue;
	}
	std::stringstream sstream(it->second);
	T value;
	sstream >> value;
	if (!sstream) {
		return defaultValue;
	}
	return value;
}

template<> std::string ConfigReader::GetOption(const std::string &name, const std::string &value) const;

} // namespace samp_profiler

#endif // !SAMP_PROFILER_CONFIGREADER_H
