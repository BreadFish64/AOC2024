namespace {
[[maybe_unused]] constexpr std::string_view test = R"(r, wr, b, g, bwu, rb, gb, br

brwrr
bggr
gbbr
rrbgbr
ubwu
bwurrg
brgr
bbrgwb
)";

struct State {
    std::vector<std::string_view> patterns;
    std::vector<std::string_view> designs;

    State(std::string_view input) {
        const size_t split = input.find("\n\n"sv);
        patterns           = input.substr(0, split) | Split(", "sv) | ranges::to_vector;
        designs            = input.substr(split + 2) | Split('\n') | ranges::to_vector;
        ranges::sort(patterns);
        
        auto it = patterns.begin();
        while (it != patterns.end()) {
            std::string_view pattern = *it;
            it                       = patterns.erase(it);
            if (!designIsPossible(pattern)) {
                it = patterns.insert(it, pattern) + 1;
            }
        }
    }

    bool designIsPossible(std::string_view design) const {
        if (design.empty()) {
            return true;
        }
        for (std::string_view pattern : ranges::equal_range(patterns, design.front(), std::less{},
                                                            [](std::string_view pattern) { return pattern.front(); })) {
            if (pattern != design.substr(0, pattern.size())) {
                continue;
            }
            if (designIsPossible(design.substr(pattern.size()))) {
                return true;
            }
        }
        return false;
    }

    size_t part1() const {
        return ranges::count_if(designs, [this](std::string_view design) { return designIsPossible(design); });
    }
};

} // namespace

void AocMain(std::string_view input) {
    //input = test;
    State state{input};
    logger.solution("Part 1: {}", StopWatch<std::micro>::Run("Part 1", &State::part1, state));
}