#define ICL_USE_AOC_IMPLEMENTATION
namespace AocIcl {
template <typename Key, typename Compare, typename Allocator>
using set = boost::container::flat_set<Key, Compare, Allocator>;
template <typename Key, typename T, typename Compare, typename Allocator>
using map = boost::container::flat_map<Key, T, Compare, Allocator>;
} // namespace AocIcl

#include <boost/icl/interval_set.hpp>

namespace {
using CoordinateInterval = boost::icl::open_interval<s64>;
using OccupiedSet        = boost::icl::interval_set<s64, std::less, CoordinateInterval>;

s64 Distance(const OccupiedSet& occupied, s64 lower, s64 upper, s64 expansion) {
    if (upper == lower) return 0;
    if (upper < lower) std::swap(lower, upper);
    CoordinateInterval distanceInterval{lower, upper};
    auto [occupiedBegin, occupiedEnd] = occupied.equal_range(distanceInterval);
    // Faster than using length(occupied & distanceInterval) which allocates a new set
    const s64 occupiedCount = ranges::accumulate(occupiedBegin, occupiedEnd, s64{}, std::plus{},
                                                 [distanceInterval](const CoordinateInterval& occupiedInterval) {
                                                     return boost::icl::length(distanceInterval & occupiedInterval);
                                                 });
    return expansion * (boost::icl::length(distanceInterval) - occupiedCount) + occupiedCount + 1;
}
} // namespace

void AocMain(std::string_view input) {
    std::vector<std::pair<s64, s64>> galaxies;
    OccupiedSet occupiedRows;
    OccupiedSet occupiedColumns;
    {
        StopWatch<std::micro> parseWatch{"Parse"};
        s64 stride = input.find('\n') + 1;
        for (s64 i = 0; i < std::ssize(input); ++i) {
            if ('#' == input[i]) {
                const auto [y, x] = std::div(i, stride);
                galaxies.emplace_back(y, x);
                occupiedRows.insert(y);
                occupiedColumns.insert(x);
            }
        }
    }
    for (s64 expansion : {2, 10, 100, 1'000'000}) {
        s64 sum{};
        {
            StopWatch<std::micro> solveWatch{fmt::format("Solve for expansion = {}", expansion)};
            for (auto lhs = galaxies.begin(); lhs != galaxies.end(); ++lhs) {
                for (auto rhs = lhs + 1; rhs != galaxies.end(); ++rhs) {
                    auto [ly, lx] = *lhs;
                    auto [ry, rx] = *rhs;
                    sum += Distance(occupiedRows, ry, ly, expansion);
                    sum += Distance(occupiedColumns, rx, lx, expansion);
                }
            }
        }
        logger.solution("{}: {}", expansion, sum);
    }
}