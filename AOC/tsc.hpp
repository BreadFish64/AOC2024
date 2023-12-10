#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Timer class
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <chrono>
#include <stdint.h>

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <cpuid.h>
#include <x86intrin.h>
#endif

#include <boost/multiprecision/cpp_int.hpp>

using DoubleSeconds = std::chrono::duration<double>;
using DoubleMs      = std::chrono::duration<double, std::milli>;

struct TSC {
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
    /// <summary>
    /// Tick rate of the hardware clock
    /// Usually the processor's advertised base clock
    /// </summary>
    static const inline TickRate TICK_RATE = GetTickRateRaw();

    template <typename Duration>
    static Duration TicksTo(const uint64_t ticks) {
        const DoubleSeconds seconds{ticks * TICK_RATE.invPerSecond};
        return std::chrono::duration_cast<Duration>(seconds);
    }

    /// <summary>
    /// Get the current TSC value
    /// </summary>
    /// <returns></returns>
    static uint64_t GetTicks() {
        _mm_mfence();     // ensure instruction ordering
        return __rdtsc(); // read the hardware clock
    }
};
