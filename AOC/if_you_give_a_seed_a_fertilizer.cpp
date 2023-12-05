using SeedRange = boost::icl::right_open_interval<s64>;
using Map = boost::icl::interval_map<s64, s64, boost::icl::partial_enricher, std::less, boost::icl::inplace_identity,
                                     boost::icl::inter_section, SeedRange>;
using SparseSeedRange = boost::icl::interval_set<s64, std::less, SeedRange>;

namespace {

void Part1(std::span<const s64> seeds, std::span<const Map> maps) {
    std::vector<s64> locations{seeds.begin(), seeds.end()};
    for (const Map& map : maps) {
        for (s64& location : locations) {
            auto mapping = map.find(location);
            if (mapping != map.end()) {
                const auto& [srcInterval, dstStart] = *mapping;
                location                            = location - srcInterval.lower() + dstStart;
            }
        }
    }
    fmt::print("Part 1: {}\n", ranges::min(locations));
}

void Part2(std::span<const s64> seeds, std::span<const Map> maps) {
    SparseSeedRange sourceSeeds;
    for (auto seedBound = seeds.begin(); seedBound != seeds.end();) {
        s64 start  = *seedBound++;
        s64 length = *seedBound++;
        sourceSeeds.insert({start, start + length});
    }
    for (const Map& map : maps) {
        SparseSeedRange dstSeeds;
        for (const auto& sourceSeedRange : sourceSeeds) {
            const auto [beginDst, endDst] = map.equal_range(sourceSeedRange);

            SparseSeedRange leftovers;
            for (const auto& [srcInterval, dstStart] : ranges::subrange(beginDst, endDst)) {
                const auto srcSeedInterval = srcInterval & sourceSeedRange;
                leftovers.insert(leftovers.end(), srcSeedInterval);
                SeedRange dstSeedInterval{srcSeedInterval.lower() - srcInterval.lower() + dstStart,
                                          srcSeedInterval.upper() - srcInterval.lower() + dstStart};
                dstSeeds.insert(dstSeedInterval);
            }
            dstSeeds += leftovers ^ sourceSeedRange;
        }
        sourceSeeds = std::move(dstSeeds);
    }
    fmt::print("Part 2: {}\n", sourceSeeds.begin()->lower());
}

Map::segment_type ParseMapLine(std::string_view map_line) {
    auto nums = map_line | ParseNumbers<s64> | ranges::to<boost::container::static_vector<s64, 3>>;
    return Map::segment_type{Map::interval_type{nums[1], nums[1] + nums[2]}, nums[0]};
}

} // namespace

void AocMain(std::string input) {
    const auto lines      = input | Split('\n');
    auto seeds            = ranges::front(lines) | views::drop(7) | ParseNumbers<s64> | ranges::to<std::vector>;
    std::vector<Map> maps = lines | views::drop(2) | views::split(""sv) | views::transform([](const auto& map_lines) {
                                Map map;
                                for (std::string_view map_line : map_lines | views::drop(1)) {
                                    map.insert(ParseMapLine(map_line));
                                }
                                return map;
                            }) |
                            ranges::to<std::vector>;
    Part1(seeds, maps);
    Part2(seeds, maps);
}