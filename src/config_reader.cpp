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

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>

#include "config_reader.h"

namespace samp_profiler {

ConfigReader::ConfigReader(const std::string &filename) 
	: loaded_(false)
{
	LoadFile(filename);
}

bool ConfigReader::LoadFile(const std::string &filename) {
	std::ifstream cfg(filename.c_str());

	if (cfg.is_open()) {
		loaded_ = true;

		std::string line;
		while (std::getline(cfg, line, '\n')) {
			std::stringstream linestream(line);

			// Get first word in the line
			std::string name;
			std::getline(linestream, name, ' ');

			// Get the rest
			std::string value;
			std::getline(linestream, value, '\n');

			options_[name] = value;
		}
	}

	return loaded_;
}

template<>
std::string ConfigReader::GetOption<std::string>(const std::string &name, const std::string &defaultValue) const {
	std::map<std::string, std::string>::const_iterator it = options_.find(name);
	if (it == options_.end()) {
		return defaultValue;
	}
	return it->second;
}

} // namespace samp_profiler
