namespace {

constexpr std::array<Pos2D, 128> DIGIT_TO_POS = [] {
    std::array<Pos2D, 128> init{};
    init['7'] = {0, 0};
    init['8'] = {0, 1};
    init['9'] = {0, 2};

    init['4'] = {1, 0};
    init['5'] = {1, 1};
    init['6'] = {1, 2};

    init['1'] = {2, 0};
    init['2'] = {2, 1};
    init['3'] = {2, 2};

    init['0'] = {3, 1};
    init['A'] = {3, 2};
    return init;
}();

constexpr std::array<Pos2D, 128> ARROW_TO_POS = [] {
    std::array<Pos2D, 128> init{};
    init['^'] = {0, 1};
    init['A'] = {0, 2};

    init['<'] = {1, 0};
    init['v'] = {1, 1};
    init['>'] = {1, 2};

    return init;
}();

void MoveY(std::string& out, Pos2D robotPos, Pos2D targetPos) {
    if (robotPos.y() > targetPos.y()) {
        out.insert(out.end(), robotPos.y() - targetPos.y(), '^');
    } else {
        out.insert(out.end(), targetPos.y() - robotPos.y(), 'v');
    }
}

void MoveX(std::string& out, Pos2D robotPos, Pos2D targetPos) {
    if (robotPos.x() > targetPos.x()) {
        out.insert(out.end(), robotPos.x() - targetPos.x(), '<');
    } else {
        out.insert(out.end(), targetPos.x() - robotPos.x(), '>');
    }
}

std::string MoveYX(Pos2D robotPos, Pos2D targetPos) {
    std::string out;
    MoveY(out, robotPos, targetPos);
    MoveX(out, robotPos, targetPos);
    out.push_back('A');
    return out;
}

std::string MoveXY(Pos2D robotPos, Pos2D targetPos) {
    std::string out;
    MoveX(out, robotPos, targetPos);
    MoveY(out, robotPos, targetPos);
    out.push_back('A');
    return out;
}

struct Solver {
    std::vector<boost::unordered_flat_map<std::pair<char, char>, size_t>> cache;

    Solver(size_t layers) : cache(layers + 1) {}

    size_t expandMove(char current, char next, size_t layer) {
        if (auto cachedIt = cache[layer].find({current, next}); cachedIt != cache[layer].end()) {
            return cachedIt->second;
        }
        auto calc = [&] {
            if (layer == 0) {
                // numpad
                const Pos2D robotPos{DIGIT_TO_POS[current]};
                const Pos2D targetPos{DIGIT_TO_POS[next]};
                if (robotPos.x() == 0 && targetPos.y() == 3) {
                    return expandedLen(MoveXY(robotPos, targetPos), layer + 1);
                } else if (robotPos.y() == 3 && targetPos.x() == 0) {
                    return expandedLen(MoveYX(robotPos, targetPos), layer + 1);
                } else {
                    size_t lenYX = expandedLen(MoveYX(robotPos, targetPos), layer + 1);
                    size_t lenXY = expandedLen(MoveXY(robotPos, targetPos), layer + 1);
                    return std::min(lenYX, lenXY);
                }
            } else {
                // arrow pad
                const Pos2D robotPos{ARROW_TO_POS[current]};
                const Pos2D targetPos{ARROW_TO_POS[next]};
                if (robotPos.x() == 0 && targetPos.y() == 0) {
                    return expandedLen(MoveXY(robotPos, targetPos), layer + 1);
                } else if (robotPos.y() == 0 && targetPos.x() == 0) {
                    return expandedLen(MoveYX(robotPos, targetPos), layer + 1);
                } else {
                    size_t lenYX = expandedLen(MoveYX(robotPos, targetPos), layer + 1);
                    size_t lenXY = expandedLen(MoveXY(robotPos, targetPos), layer + 1);
                    return std::min(lenYX, lenXY);
                }
            }
        };
        return cache[layer][{current, next}] = calc();
    }

    size_t expandedLen(std::string_view input, size_t layer) {
        if (layer == cache.size()) {
            return input.size();
        } else {
            size_t len{0};
            char current{'A'};
            for (char next : input) {
                len += expandMove(current, next, layer);
                current = next;
            }
            return len;
        }
    }

    size_t complexity(std::string_view input) {
        size_t num        = ParseNumber<size_t>(input.substr(0, input.size() - 1));
        size_t minLen     = expandedLen(input, 0);
        size_t complexity = minLen * num;
        logger.info("{}: {} * {} = {}", input, minLen, num, complexity);
        return complexity;
    }

    size_t complexitySum(auto&& codes) {
        return ranges::accumulate(codes, size_t{}, std::plus{},
                                  [this](std::string_view code) { return complexity(code); });
    }
};

} // namespace

void AocMain(std::string_view input) {
    auto codes = input | Split('\n');
    logger.solution("Part 1: {}",
                    StopWatch<std::micro>::Run("2 layers", [&codes] { return Solver{2}.complexitySum(codes); }));
    logger.solution("Part 2: {}",
                    StopWatch<std::micro>::Run("25 layers", [&codes] { return Solver{25}.complexitySum(codes); }));
}