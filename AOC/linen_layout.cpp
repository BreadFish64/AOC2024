namespace {

struct State {
    std::array<std::vector<std::string_view>, 128> rawPatterns;
    mutable boost::unordered_flat_map<std::string_view, size_t> patternCache;
    std::vector<std::string_view> designs;

    State(std::string_view input) {
        const size_t split = input.find("\n\n"sv);
        designs            = input.substr(split + 2) | Split('\n') | ranges::to_vector;

        for (std::string_view pattern : input.substr(0, split) | Split(", "sv)) {
            rawPatterns[pattern.front()].emplace_back(pattern);
        }
        for (auto& rawPatternVec : rawPatterns) {
            // Not really required but is faster
            ranges::sort(rawPatternVec, std::greater{});
        }
    }

    bool designIsPossible(std::string_view design) const {
        if (design.empty()) {
            return true;
        }
        for (std::string_view pattern : rawPatterns[design.front()]) {
            if (!design.starts_with(pattern)) {
                continue;
            }
            if (designIsPossible(design.substr(pattern.size()))) {
                return true;
            }
        }
        return false;
    }

    size_t possibleArrangements(std::string_view design) const {
        if (design.empty()) {
            return 1;
        }
        {
            auto it = patternCache.find(design);
            if (it != patternCache.end()) {
                return it->second;
            }
        }
        size_t possibleArrangementCount{0};
        for (std::string_view pattern : rawPatterns[design.front()]) {
            if (!design.starts_with(pattern)) {
                continue;
            }
            possibleArrangementCount += possibleArrangements(design.substr(pattern.size()));
        }
        patternCache.emplace(design, possibleArrangementCount);
        return possibleArrangementCount;
    }

    size_t part1() const {
        return ranges::count_if(designs, [this](std::string_view design) {
            logger.info("{}", design);
            logger.flush();
            return designIsPossible(design);
        });
    }

    size_t part2() const {
        return ranges::accumulate(designs, size_t{0}, std::plus{},
                                  [this](std::string_view design) { return possibleArrangements(design); });
    }
};

} // namespace

void AocMain(std::string_view input) {
    State state{input};
    logger.solution("Part 2: {}", StopWatch<std::micro>::Run("Part 2", &State::part2, state));
    logger.solution("Part 1: {}", StopWatch<std::micro>::Run("Part 1", &State::part1, state));
}