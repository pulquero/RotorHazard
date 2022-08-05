#include "microclock.h"

MicroClock usclock;

#if TARGET == STM32_TARGET
#define USE_NATIVE_TIMER
#endif


utime_t MicroClock::tickMicros()
{
#ifndef USE_NATIVE_TIMER
    const utime_t currentTick = ::micros();
    // unsigned arithmetic to handle roll-over
    uint32_t elapsed = currentTick - prevTick;
    uint32_t delta = elapsed + excessMicros;
    uint32_t deltaMillis = delta/1000;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        timeMillis += deltaMillis;
        excessMicros = delta - deltaMillis*1000;
    }
    prevTick = currentTick;
    return currentTick;
#else
    return ::micros();
#endif
}

mtime_t MicroClock::millis()
{
#ifndef USE_NATIVE_TIMER
    return timeMillis;
#else
    // native impl is sufficiently accurate
    return ::millis();
#endif
}

void MicroClock::delay(mtime_t ms)
{
    const utime_t duration = 1000*ms;
    const utime_t now = tickMicros();
    while (tickMicros() - now < duration) {
        delayMicroseconds(500);
    }
}
