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

#ifndef JUMP_X86_H
#define JUMP_X86_H

namespace samp_profiler {

class JumpX86 {
public:
	static const int kJmpInstrSize = 5;

	JumpX86();
	JumpX86(void *src, void *dst);
	~JumpX86();

	bool Install();
	bool Install(void *src, void *dst);
	bool Remove();

	bool IsInstalled() const;

private:
	void *src_;
	void *dst_;
	unsigned char code_[5];
	bool installed_;
};

} // namespace samp_profiler

#endif

