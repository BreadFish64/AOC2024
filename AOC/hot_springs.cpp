namespace {
struct SpringState {
    u128 springs;
    u128 unknowns;
    std::span<const u8> expectedGroups;

    friend bool operator==(const SpringState& lhs, const SpringState& rhs) {
        return lhs.springs == rhs.springs && lhs.unknowns == rhs.unknowns &&
               lhs.expectedGroups.size() == rhs.expectedGroups.size();
    }
    u128 combined() const { return springs | unknowns; }
    SpringState operator>>(int shift) const {
        return SpringState{
            .springs        = springs >> shift,
            .unknowns       = unknowns >> shift,
            .expectedGroups = expectedGroups,
        };
    }
    SpringState trim() const {
        const u32 trim = countr_zero(combined());
        return *this >> trim;
    }
    SpringState skipGroup(u32 length) const {
        SpringState ret    = *this >> (length + 1);
        ret.expectedGroups = ret.expectedGroups.subspan<1>();
        return ret;
    }
    SpringState insertSpring(u32 position) const {
        const u128 mask = u128{1} << position;
        return SpringState{
            .springs        = springs | mask,
            .unknowns       = unknowns & ~mask,
            .expectedGroups = expectedGroups,
        };
    }
    SpringState insertSpace(u32 position) const {
        const u128 mask = u128{1} << position;
        return SpringState{
            .springs        = springs,
            .unknowns       = unknowns & ~mask,
            .expectedGroups = expectedGroups,
        };
    }
};
struct SpringStateHash {
    size_t operator()(const SpringState& x) const {
        size_t seed{};
        HashCombine(seed, x.springs);
        HashCombine(seed, x.unknowns);
        HashCombine(seed, x.expectedGroups.size());
        return seed;
    }
};
struct SpringRow {
    u32 springs;
    u32 unknowns;
    std::vector<u8> groups;
    u8 length;

private:
    static u64 permutationsInternal(SpringState state,
                                    boost::unordered_flat_map<SpringState, u64, SpringStateHash>& cache) {
        state = state.trim();
        Assume(0 == (state.springs & state.unknowns));
        if (state.expectedGroups.empty()) {
            return (state.springs == 0) ? 1 : 0;
        }
        const u128 combined              = state.combined();
        const u32 nextGroup              = state.expectedGroups.front();
        const u32 springsFirstGroupSize  = countr_one(state.springs);
        const u32 unknownsFirstGroupSize = countr_one(state.unknowns);
        const u32 combinedFirstGroupSize = countr_one(combined);
        const u32 requiredUnknowns       = ranges::accumulate(state.expectedGroups, u32{}) - popcount(state.springs);
        const u32 remainingUnknowns      = popcount(state.unknowns);
        if (combined == 0) {
            return 0;
        }
        if (springsFirstGroupSize > nextGroup) {
            return 0;
        }
        if (combinedFirstGroupSize < nextGroup) {
            if (unknownsFirstGroupSize == combinedFirstGroupSize) {
                return permutationsInternal(state >> unknownsFirstGroupSize, cache);
            } else {
                return 0;
            }
        }
        if (remainingUnknowns < requiredUnknowns) {
            return 0;
        }
        if (springsFirstGroupSize == nextGroup) {
            return permutationsInternal(state.skipGroup(springsFirstGroupSize), cache);
        }
        if (auto it = cache.find(state); it != cache.end()) {
            return it->second;
        }
        u32 flipPosition    = countr_zero(state.unknowns);
        return cache[state] = permutationsInternal(state.insertSpring(flipPosition), cache) +
                              permutationsInternal(state.insertSpace(flipPosition), cache);
    }

public:
    u64 permutations(const int folds) const {
        std::vector<u8> expandedGroups;
        for (int fold = 0; fold < folds; ++fold) {
            expandedGroups.insert(expandedGroups.end(), groups.begin(), groups.end());
        }
        SpringState state{.expectedGroups = std::span{expandedGroups}};
        for (int fold = 0; fold < folds; ++fold) {
            state.unknowns <<= length;
            state.unknowns |= unknowns;
            state.unknowns <<= 1;
            state.unknowns |= 1;

            state.springs <<= length;
            state.springs |= springs;
            state.springs <<= 1;
        }
        state.unknowns >>= 1;
        state.springs >>= 1;
        boost::unordered_flat_map<SpringState, u64, SpringStateHash> cache;
        return permutationsInternal(state, cache);
    }
};

u64 Solve(std::span<const SpringRow> springRows, const int folds) {
    return ranges::accumulate(springRows, u64{}, std::plus{},
                              [folds](const SpringRow& springRow) { return springRow.permutations(folds); });
}

} // namespace

void AocMain(std::string_view input) {
    std::vector<SpringRow> springRows = input | Split('\n') | views::transform([](std::string_view line) {
                                            const size_t space = line.find(' ');
                                            SpringRow springRow{
                                                .groups = line.substr(space + 1) | Split(',') |
                                                          views::transform(ParseNumber<u8>) | ranges::to<std::vector>,
                                                .length = static_cast<u8>(space),
                                            };
                                            auto springView = line.substr(0, space);
                                            for (size_t i = 0; i < springView.size(); ++i) {
                                                if (springView[i] == '#') springRow.springs |= 1U << i;
                                                if (springView[i] == '?') springRow.unknowns |= 1U << i;
                                            }
                                            return springRow;
                                        }) |
                                        ranges::to<std::vector>;
    logger.solution("Part 1: {}\n", StopWatch<std::milli>::Run("Part1", Solve, springRows, 1));
    logger.solution("Part 2: {}\n", StopWatch<std::milli>::Run("Part2", Solve, springRows, 5));
}

// template <int FOLDS = 5>
// u64 Solve(std::span<const SpringRow> springRows) {
//     std::atomic<u64> totalPermutations{};
//     std::atomic<size_t> rowIndex{};
//     std::vector<std::future<void>> workers(std::thread::hardware_concurrency());
//     for (auto& worker : workers) {
//         worker = std::async(std::launch::async, [&] {
//             size_t idx{};
//             while ((idx = rowIndex.fetch_add(1)) < springRows.size()) {
//                 auto startTime          = std::chrono::steady_clock::now();
//                 const SpringRow row     = springRows[idx];
//                 const u32 totalUnknowns = std::popcount(row.unknowns) * FOLDS + FOLDS - 1;
//                 const u32 requiredUnknowns =
//                     FOLDS * (ranges::accumulate(row.groups, u32{}) - std::popcount(row.springs));
//                 boost::container::static_vector<u8, 6 * FOLDS> groups;
//                 for (int fold = 0; fold < FOLDS; ++fold) {
//                     groups.insert(groups.end(), row.groups.begin(), row.groups.end());
//                 }
//                 u128 springs{};
//                 u128 unknowns{};
//                 for (int fold = 0; fold < FOLDS; ++fold) {
//                     unknowns <<= row.length;
//                     unknowns |= row.unknowns;
//                     unknowns <<= 1;
//                     unknowns |= 1;
//                     springs <<= row.length;
//                     springs |= row.springs;
//                     springs <<= 1;
//                 }
//                 unknowns >>= 1;
//                 springs >>= 1;
//                 {
//                     u128 combined = springs | unknowns;
//                     u32 maxGroupSize{0};
//
//                     for (const u32 groupSize : groups) {
//                         combined >>= countr_zero(combined);
//                         u32 candidateGroupSize = countr_one(combined);
//                         combined >>= candidateGroupSize;
//                         maxGroupSize = std::max(maxGroupSize, candidateGroupSize);
//                     }
//                     fmt::print("Max Group Size: {}\n", maxGroupSize);
//                 }
//                 continue;
//                 u64 unknownsLo{static_cast<u64>(unknowns)};
//                 u64 unknownsHi{static_cast<u64>(unknowns >> 64)};
//                 const u32 lowDeposits = std::popcount(unknownsLo);
//                 fmt::print("idx: {} - unknowns: {} - required placements: {}\n", idx, totalUnknowns,
//                 requiredUnknowns); const u128 stop = u128{1} << totalUnknowns; u64 workingPermutations{}; for (u128
//                 permutation = (u128{1} << requiredUnknowns) - 1; permutation < stop;
//                      permutation      = bit_twiddle_permute(permutation)) {
//                     if ([&] {
//                             u128 candidate = springs;
//                             candidate |= _pdep_u64(permutation, unknownsLo);
//                             candidate |= u128{_pdep_u64(permutation >> lowDeposits, unknownsHi)} << 64;
//                             for (const u32 groupSize : groups) {
//                                 candidate >>= countr_zero(candidate);
//                                 u32 candidateGroupSize = countr_one(candidate);
//                                 if (candidateGroupSize != groupSize) [[likely]] {
//                                     return false;
//                                 }
//                                 candidate >>= candidateGroupSize;
//                             }
//                             return true;
//                         }()) [[unlikely]] {
//                         ++workingPermutations;
//                     }
//                 }
//                 auto stopTime = std::chrono::steady_clock::now();
//                 fmt::print("idx: {} - working: {} - {}\n", idx, workingPermutations,
//                            DoubleSeconds{stopTime - startTime});
//                 totalPermutations += workingPermutations;
//             }
//         });
//     }
//     for (auto& worker : workers)
//         worker.wait();
//     return totalPermutations;
// }
