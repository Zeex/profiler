// Copyright (c) 2011-2012, Zeex
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met: 
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer. 
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution. 
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// // LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef AMX_PROFILER_FUNCTION_CALL_H
#define AMX_PROFILER_FUNCTION_CALL_H

#include <memory>
#include <amx/amx.h>
#include "function.h"
#include "performance_counter.h"

namespace amx_profiler {

class FunctionCall {
public:
	FunctionCall(const std::shared_ptr<Function> &function,
	             cell frame,
	             const std::shared_ptr<FunctionCall> &parent = 0);

	std::shared_ptr<Function> &function()
		{ return fn_; }
	const std::shared_ptr<Function> &function() const
		{ return fn_; }

	std::shared_ptr<FunctionCall> &parent()
		{ return parent_; }
	const std::shared_ptr<FunctionCall> &parent() const
		{ return parent_; }

	cell frame() const
		{ return frame_; }

	PerformanceCounter &timer()
		{ return timer_; }
	const PerformanceCounter &timer() const
		{ return timer_; }

	bool IsRecursive() const
		{ return recursive_; }

private:
	std::shared_ptr<Function> fn_;
	std::shared_ptr<FunctionCall> parent_;
	cell frame_; // frame address on AMX stack
	PerformanceCounter timer_;
	bool recursive_; // whether it's a recursive call
};

} // namespace amx_profiler

#endif // !AMX_PROFILER_FUNCTION_CALL_H
