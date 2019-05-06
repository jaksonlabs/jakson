/**
 * Copyright 2019 Marcus Pinnecke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "utils/env.h"

#if defined(_WIN32)
#include <Windows.h>
#include <Psapi.h>
#elif defined(__APPLE__)
#include <mach/mach.h>
#include <mach/task.h>
#else
#include <unistd.h>
#include <sys/resource.h>
#endif

NG5_EXPORT(size_t) env_get_process_memory_usage() {
#if defined(_WIN32)
        PROCESS_MEMORY_COUNTERS counters;
	memset(&counters, 0, sizeof(counters));
	counters.cb = sizeof(counters);
	GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters));
	return counters.WorkingSetSize;
#elif defined(__APPLE__)
        struct task_basic_info info;
        mach_msg_type_number_t info_count = TASK_BASIC_INFO_COUNT;
        if (task_info(mach_task_self(), TASK_BASIC_INFO,
                (task_info_t)&info, &info_count) != KERN_SUCCESS)
                return 0;
        return info.resident_size;
#else
        long rss = 0L;
	FILE* fp = fopen("/proc/self/statm", "r");
	if (!fp)
		return 0;
	if (fscanf(fp, "%*s%ld", &rss) != 1)
		rss = 0;
	fclose(fp);
	return (size_t)rss * (size_t)sysconf(_SC_PAGESIZE);
#endif
}

NG5_EXPORT(size_t) env_get_process_peak_memory_usage() {
#if defined(_WIN32)
        PROCESS_MEMORY_COUNTERS counters;
	memset(&counters, 0, sizeof(counters));
	counters.cb = sizeof(counters);
	GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters));
	return counters.PeakWorkingSetSize;
#else
        struct rusage rusage;
        getrusage(RUSAGE_SELF, &rusage);
#if defined(__APPLE__)
        return (size_t)rusage.ru_maxrss;
#else
        return (size_t)rusage.ru_maxrss * 1024;
#endif
#endif
}
