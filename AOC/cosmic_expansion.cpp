namespace {

void CompactGalaxies(boost::container::static_vector<u32, 192>& compacted,
                     boost::container::static_vector<u32, 192>& counts, const std::span<const u32> galaxies) {
    compacted.emplace_back(galaxies.front());
    counts.emplace_back(1);
    for (const u32 galaxy : galaxies.subspan<1>()) {
        if (galaxy == compacted.back()) {
            ++counts.back();
        } else {
            compacted.emplace_back(galaxy);
            counts.emplace_back(1);
        }
    }
    counts.resize(counts.size() + 8);
}

u64 SolveDimension(boost::container::static_vector<u32, 192> galaxies, const std::span<const u32> counts,
                   const u32 expansion) {
    {
        u32 lastGalaxyOld{galaxies.front()};
        u32 lastGalaxyExpanded{galaxies.front()};
        for (u32& __restrict g : std::span{galaxies}.subspan<1>()) {
            lastGalaxyExpanded += expansion * ((g - std::exchange(lastGalaxyOld, g)) - 1) + 1;
            g = lastGalaxyExpanded;
        }
    }
    const size_t galaxyCount{galaxies.size()};
    galaxies.resize(galaxies.size() + 8); // Add padding to avoid masking
    u64 sum{0};
    __m256i vSumLo = _mm256_setzero_si256();
    __m256i vSumHi = _mm256_setzero_si256();
    for (size_t lower = 0; lower < galaxyCount - 1; ++lower) {
        __m256i vLowerCount  = _mm256_set1_epi32(counts[lower]);
        __m256i vLowerGalaxy = _mm256_set1_epi32(galaxies[lower]);
        for (size_t upper = lower + 1; upper < galaxyCount; upper += 8) {
            __m256i vUpperCounts   = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(counts.data() + upper));
            __m256i vUpperGalaxies = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(galaxies.data() + upper));
            __m256i vCombinations  = _mm256_mullo_epi32(vUpperCounts, vLowerCount);
            __m256i vDistances     = _mm256_sub_epi32(vUpperGalaxies, vLowerGalaxy);
            if constexpr (true) {
                vSumLo = _mm256_add_epi64(vSumLo, _mm256_mul_epu32(vCombinations, vDistances));
                vSumHi = _mm256_add_epi64(
                    vSumHi, _mm256_mul_epu32(_mm256_srli_epi64(vCombinations, 32), _mm256_srli_epi64(vDistances, 32)));
            } else {
                __m256i tmp32 = _mm256_mullo_epi32(vCombinations, vDistances);
                vSumLo        = _mm256_add_epi64(vSumLo, _mm256_cvtepu32_epi64(_mm256_castsi256_si128(tmp32)));
                vSumHi        = _mm256_add_epi64(vSumHi, _mm256_cvtepu32_epi64(_mm256_extracti128_si256(tmp32, 1)));
            }
        }
    }
    vSumLo = _mm256_add_epi64(vSumLo, vSumHi);
    sum += _mm256_extract_epi64(vSumLo, 0);
    sum += _mm256_extract_epi64(vSumLo, 1);
    sum += _mm256_extract_epi64(vSumLo, 2);
    sum += _mm256_extract_epi64(vSumLo, 3);
    return sum;
}
} // namespace

void AocMain(std::string_view input) {
    boost::container::static_vector<u32, 512> galaxiesY;
    boost::container::static_vector<u32, 512> galaxiesX;
    boost::container::static_vector<u32, 192> compactedGalaxiesY;
    boost::container::static_vector<u32, 192> countsY;
    boost::container::static_vector<u32, 192> compactedGalaxiesX;
    boost::container::static_vector<u32, 192> countsX;
    {
        StopWatch<std::micro> parseWatch{"Parse"};
        u32 stride = input.find('\n') + 1;
        for (u32 i = 0; i < input.size(); ++i) {
            if ('#' == input[i]) {
                galaxiesY.emplace_back(i / stride);
                galaxiesX.emplace_back(i % stride);
            }
        }
        std::ranges::sort(galaxiesX);
        CompactGalaxies(compactedGalaxiesX, countsX, galaxiesX);
        CompactGalaxies(compactedGalaxiesY, countsY, galaxiesY);
    }
    for (int round = 0; round < 50; ++round) {
        for (u32 expansion : {2, 10, 100, 1'000'000}) {
            u64 sum{};
            {
                StopWatch<std::micro> solveWatch{fmt::format("Solve for expansion = {}", expansion)};
                sum = SolveDimension(compactedGalaxiesY, countsY, expansion) +
                      SolveDimension(compactedGalaxiesX, countsX, expansion);
            }
            logger.solution("{}: {}", expansion, sum);
        }
    }
}