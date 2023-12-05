namespace {
struct Round {
    int r{}, g{}, b{};
};

void Part1(std::span<std::vector<Round>> games) {
    ssize idsum = 0;
    for (const auto& [id, game] : views::enumerate(games)) {
        if (ranges::all_of(game, [](const Round& round) { return round.r <= 12 && round.g <= 13 && round.b <= 14; })) {
            idsum += id + 1;
        }
    }
    fmt::print("Part 1: {}\n", idsum);
}

void Part2(std::span<std::vector<Round>> games) {
    fmt::print("Part 2: {}\n", ranges::accumulate(games, ssize{}, std::plus{}, [](const auto& game) {
                   return ranges::max(game, std::less{}, &Round::r).r * ranges::max(game, std::less{}, &Round::g).g *
                          ranges::max(game, std::less{}, &Round::b).b;
               }));
}
} // namespace

void AocMain(std::string input) {
    std::vector<std::vector<Round>> games;
    for (std::string_view line : input | Split('\n')) {
        std::vector<Round> game;
        line.remove_prefix(line.find(' ', line.find(' ') + 1) + 1);
        for (std::string_view round_str : line | Split(';')) {
            if (round_str.front() == ' ') round_str.remove_prefix(1);
            Round round{};
            for (auto&& cube : round_str | Split(' ') | views::chunk(2)) {
                std::string_view count = *ranges::begin(cube);
                std::string_view color = *std::next(ranges::begin(cube));
                if (color.front() == 'r') {
                    std::from_chars(count.data(), count.data() + count.size(), round.r);
                } else if (color.front() == 'g') {
                    std::from_chars(count.data(), count.data() + count.size(), round.g);
                } else if (color.front() == 'b') {
                    std::from_chars(count.data(), count.data() + count.size(), round.b);
                } else {
                    assert(false);
                }
            }
            game.emplace_back(round);
        }
        games.emplace_back(std::move(game));
    }
    Part1(games);
    Part2(games);
}