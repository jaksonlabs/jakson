#include <stdx/time.h>
#include <time.h>
#include <math.h>

timestamp_t time_current_time_ms()
{
    long            ms;
    time_t          s;
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    s  = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6);
    return s * 1000 + ((ms > 999) ? 1000 : ms);
}