namespace {
constexpr size_t MAP_SIZE = 1_sz << 15;
constexpr unsigned ToIndex(std::string_view coord) {
    return ((coord[0] - 'A') << 10) + ((coord[1] - 'A') << 5) + (coord[2] - 'A');
}
constexpr unsigned ZZZ       = ToIndex("ZZZ"sv);
constexpr unsigned LAST_MASK = (1 << 5) - 1;
constexpr bool XXZ(const unsigned coord) {
    return (coord & LAST_MASK) == 25;
}
constexpr bool XXA(const unsigned coord) {
    return (coord & LAST_MASK) == 0;
}

s64 Traverse(std::string_view steps, std::span<const std::pair<u16, u16>, MAP_SIZE> map, unsigned coord,
             std::invocable<unsigned> auto&& shouldBreak) {
    s64 stepCount = 0;
    while (!shouldBreak(coord)) {
        for (const char step : steps) {
            if (shouldBreak(coord)) [[unlikely]]
                break;
            ++stepCount;
            const auto [l, r] = map[coord];
            coord             = step == 'L' ? l : r;
        }
    }
    return stepCount;
}

s64 Part1(std::string_view steps, std::span<const std::pair<u16, u16>, MAP_SIZE> map) {
    return Traverse(steps, map, 0, [](const unsigned coord) { return coord == ZZZ; });
}
s64 Part2(std::string_view steps, std::span<const std::pair<u16, u16>, MAP_SIZE> map,
                            std::span<const u16> endsInA) {
    return ranges::accumulate(endsInA, s64{1}, std::lcm<s64, s64>,
                              [&](const unsigned coord) { return Traverse(steps, map, coord, XXZ); });
}

void Parse(std::string& steps, std::span<std::pair<u16, u16>, MAP_SIZE> map,
                             boost::container::small_vector<u16, 8>& endsInA, std::string_view input) {
    auto lines = input | Split('\n');
    steps      = ranges::front(lines);
    for (std::string_view line : lines | views::drop(2)) {
        const unsigned coord = ToIndex(line.substr(0, 3));
        const unsigned l     = ToIndex(line.substr(7, 3));
        const unsigned r     = ToIndex(line.substr(12, 3));
        if (XXA(coord)) {
            endsInA.emplace_back(coord);
        }
        map[coord] = {static_cast<u16>(l), static_cast<u16>(r)};
    }
}

} // namespace

void AocMain(std::string_view input) {
    std::string steps;
    std::array<std::pair<u16, u16>, MAP_SIZE> map;
    boost::container::small_vector<u16, 8> endsInA;
    StopWatch<std::micro>::Run("Parse", Parse, steps, map, endsInA, input);
    logger.solution("Part 1: {}", StopWatch<std::micro>::Run("Part1", Part1, steps, map));
    logger.solution("Part 2: {}", StopWatch<std::micro>::Run("Part2", Part2, steps, map, endsInA));
}