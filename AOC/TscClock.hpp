#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Timer class
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <chrono>
#include <concepts>

#ifdef _MSC_VER
#include <intrin.h>
#include <Windows.h>
#else
#include <cpuid.h>
#include <x86intrin.h>
#endif

struct TscClock {
    struct TickRate {
        uint32_t crystalFrequency;
        uint32_t tscCrystalRatioNumerator;
        uint32_t tscCrystalRatioDenominator;
        double perSecond;
        double invPerSecond;

        explicit TickRate(uint32_t crystalFrequency, uint32_t tscCrystalRatioNumerator,
                          uint32_t tscCrystalRatioDenominator);
    };

private:
    static TickRate GetTickRateRaw();

public:
    using rep    = double;
    using period = std::ratio<1, 1>; // Seconds

    using duration                  = std::chrono::duration<rep, period>;
    using time_point                = std::chrono::time_point<TscClock>;
    static constexpr bool is_steady = true;

    /// <summary>
    /// Tick rate of the hardware clock
    /// Usually the processor's advertised base clock
    /// </summary>
    static const inline TickRate TICK_RATE = GetTickRateRaw();

    static uint64_t rdtsc() noexcept {
        _mm_lfence();         // ensure instruction ordering
        auto tsc = __rdtsc(); // read the hardware clock
        _mm_lfence();         // ensure instruction ordering
        return tsc;
    }

    /// <summary>
    /// Get the current TSC value
    /// </summary>
    static time_point now() noexcept {
        return time_point{duration{rdtsc() * TICK_RATE.invPerSecond}};
    }
};
