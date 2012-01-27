// AMX profiler for SA-MP server: http://sa-mp.com
//
// Copyright (C) 2011-2012 Sergey Zolotarev
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

#ifndef AMX_PROFILER_PERFORMANCE_COUNTER_H
#define AMX_PROFILER_PERFORMANCE_COUNTER_H

#include "time_interval.h"
#include "time_res.h"

namespace amx_profiler {

class PerformanceCounter {
public:
	typedef chrono::high_resolution_clock ClockType;

	PerformanceCounter(PerformanceCounter *parent = 0);
	~PerformanceCounter();

	void Start();
	void Stop();

	template<typename Resolution>
	inline TimeInterval child_time() const {
		return chrono::duration_cast<Resolution>(child_time_).count();
	}

	template<typename Resolution>
	inline TimeInterval total_time() const {
		return chrono::duration_cast<Resolution>(total_time_).count();
	}

	template<typename Resolution>
	inline TimeInterval self_time() const {
		return total_time<Resolution>() - child_time<Resolution>();
	}

private:
	bool started_;
	PerformanceCounter *parent_;
	ClockType::time_point start_point_;
	ClockType::duration child_time_;
	ClockType::duration total_time_;
};

} // namespace amx_profiler

#endif // !AMX_PROFILER_PERFORMANCE_COUNTER_H
