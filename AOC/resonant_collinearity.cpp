#include "Mdspan.hpp"
void AocMain(std::string_view input) {
    std::array<boost::container::small_vector<Pos2D, 8>, 128> frequencies{};
    auto map = ToGrid(input);
    for (Pos2D pos{}; pos.y() < map.extent(0); ++pos.y()) {
        for (pos.x() = 0; pos.x() < map.extent(1); ++pos.x()) {
            char cell = map[pos];
            if (cell != '.') {
                frequencies[cell].emplace_back(pos);
            }
        }
    }
    boost::unordered::unordered_flat_set<Pos2D> antinodes1{};
    boost::unordered::unordered_flat_set<Pos2D> antinodes2{};
    auto AddIfInBounds = [&](const Pos2D& point) {
        if (InBounds(map, point)) {
            antinodes1.emplace(point);
        }
    };
    for (const auto& antennae : frequencies) {
        for (auto p1it = antennae.begin(); p1it != antennae.end(); ++p1it) {
            const Pos2D& p1 = *p1it;
            for (const Pos2D& p2 : ranges::subrange(p1it + 1, antennae.end())) {
                const Vec2D distance = p2 - p1;
                // Part 1
                AddIfInBounds(p1 - distance);
                AddIfInBounds(p2 + distance);
                // Part 2
                const Vec2D slope = distance; // / std::gcd(distance[0], distance[1]);
                for (Pos2D point = p1; InBounds(map, point); point -= slope) {
                    antinodes2.emplace(point);
                }
                for (Pos2D point = p1 + slope; InBounds(map, point); point += slope) {
                    antinodes2.emplace(point);
                }
            }
        }
    }
    logger.solution("Part 1: {}", antinodes1.size());
    logger.solution("Part 2: {}", antinodes2.size());
}