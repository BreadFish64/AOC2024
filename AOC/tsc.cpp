#include "tsc.hpp"

TSC::TickRate::TickRate(uint32_t crystalFrequency, uint32_t tscCrystalRatioNumerator,
                        uint32_t tscCrystalRatioDenominator)
    : crystalFrequency{crystalFrequency}, tscCrystalRatioNumerator{tscCrystalRatioNumerator},
      tscCrystalRatioDenominator{tscCrystalRatioDenominator},
      perSecond{crystalFrequency * (static_cast<double>(tscCrystalRatioNumerator) / tscCrystalRatioDenominator)},
      invPerSecond{static_cast<double>(tscCrystalRatioDenominator) / tscCrystalRatioNumerator / crystalFrequency} {}

TSC::TickRate TSC::GetTickRateRaw() {
    int info[4]{};
#ifdef _MSC_VER
    __cpuid(info, 0x15);
#else
    __cpuid(0x15, info[0], info[1], info[2], info[3]);
#endif
    const uint32_t tsc_crystal_ratio_denominator = info[0];
    const uint32_t tsc_crystal_ratio_numerator   = info[1];
    const uint32_t crystal_frequency             = info[2];
    if (!(crystal_frequency && tsc_crystal_ratio_numerator && tsc_crystal_ratio_denominator)) {
        throw std::runtime_error{"TSC tick rate could not be determined"};
    }
    return TickRate{crystal_frequency, tsc_crystal_ratio_numerator, tsc_crystal_ratio_denominator};
}
