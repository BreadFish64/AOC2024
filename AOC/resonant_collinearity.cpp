void AocMain(std::string_view input) {
    std::array<boost::container::small_vector<Index2D, 8>, 128> frequencies{};
    auto map = ToGrid(input);
    for (int32_t y{0}; y < map.extent(0); ++y) {
        for (int32_t x{0}; x < map.extent(1); ++x) {
            char cell = map(y, x);
            if (cell != '.') {
                frequencies[cell].emplace_back(y, x);
            }
        }
    }
    boost::unordered::unordered_flat_set<Index2D> antinodes1{};
    boost::unordered::unordered_flat_set<Index2D> antinodes2{};
    auto AddIfInBounds = [&](const Index2D& point) {
        if (InBounds(map, point)) {
            antinodes1.emplace(point);
        }
    };
    for (const auto& antennae : frequencies) {
        for (auto p1it = antennae.begin(); p1it != antennae.end(); ++p1it) {
            const Index2D& p1 = *p1it;
            for (const Index2D& p2 : ranges::subrange(p1it + 1, antennae.end())) {
                const Index2D distance = p2 - p1;
                // Part 1
                AddIfInBounds(p1 - distance);
                AddIfInBounds(p2 + distance);
                // Part 2
                const Index2D slope = distance; // / std::gcd(distance[0], distance[1]);
                for (Index2D point = p1; InBounds(map, point); point -= slope) {
                    antinodes2.emplace(point);
                }
                for (Index2D point = p1 + slope; InBounds(map, point); point += slope) {
                    antinodes2.emplace(point);
                }
            }
        }
    }
    logger.solution("Part 1: {}", antinodes1.size());
    logger.solution("Part 2: {}", antinodes2.size());
}