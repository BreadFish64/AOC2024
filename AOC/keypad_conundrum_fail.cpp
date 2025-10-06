#include <print>

namespace {
[[maybe_unused]] constexpr std::string_view test = R"(029A
980A
179A
456A
379A
)";

using Instructions = boost::container::flat_map<std::string, size_t>;

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

Pos2D MoveY(std::string& out, Pos2D robotPos, Pos2D targetPos) {
    while (robotPos.y() > targetPos.y()) {
        out.push_back('^');
        --robotPos.y();
    }
    while (robotPos.y() < targetPos.y()) {
        out.push_back('v');
        ++robotPos.y();
    }
    return robotPos;
}

Pos2D MoveX(std::string& out, Pos2D robotPos, Pos2D targetPos) {
    while (robotPos.x() > targetPos.x()) {
        out.push_back('<');
        --robotPos.x();
    }
    while (robotPos.x() < targetPos.x()) {
        out.push_back('>');
        ++robotPos.x();
    }
    return robotPos;
}

using Set = boost::container::flat_set<Instructions>;

Set NumToArrow(std::string_view input) {
    Set outs{Instructions{}};
    Set nextOuts{};
    Pos2D robotPos{DIGIT_TO_POS['A']};
    for (const char digit : input) {
        const Pos2D targetPos{DIGIT_TO_POS[digit]};
        std::string out;

        auto push = [&](std::string toAdd) {
            toAdd.push_back('A');
            for (const Instructions& in : outs) {
                Instructions add{in};
                ++add[toAdd];
                nextOuts.emplace(add);
            }
        };

        if (robotPos.x() == 0 && targetPos.y() == 3) {
            robotPos = MoveY(out, MoveX(out, robotPos, targetPos), targetPos);
            push(std::move(out));
        } else if (robotPos.y() == 3 && targetPos.x() == 0) {
            robotPos = MoveX(out, MoveY(out, robotPos, targetPos), targetPos);
            push(std::move(out));
        } else {
            MoveX(out, MoveY(out, robotPos, targetPos), targetPos);
            push(std::move(out));
            out.clear();
            robotPos = MoveY(out, MoveX(out, robotPos, targetPos), targetPos);
            push(std::move(out));
        }

        outs.swap(nextOuts);
        nextOuts.clear();
    }
    return outs;
}

Set ArrowToArrowInner(std::string_view input) {
    Set outs{Instructions{}};
    Set nextOuts{};
    Pos2D robotPos{ARROW_TO_POS['A']};
    for (const char digit : input) {
        const Pos2D targetPos{ARROW_TO_POS[digit]};
        std::string out;

        auto push = [&](std::string toAdd) {
            toAdd.push_back('A');
            for (const Instructions& in : outs) {
                Instructions add{in};
                ++add[toAdd];
                nextOuts.emplace(add);
            }
        };

        if (robotPos.x() == 0 && targetPos.y() == 0) {
            robotPos = MoveY(out, MoveX(out, robotPos, targetPos), targetPos);
            push(std::move(out));
        } else if (robotPos.y() == 0 && targetPos.x() == 0) {
            robotPos = MoveX(out, MoveY(out, robotPos, targetPos), targetPos);
            push(std::move(out));
        } else {
            MoveX(out, MoveY(out, robotPos, targetPos), targetPos);
            push(std::move(out));
            out.clear();
            robotPos = MoveY(out, MoveX(out, robotPos, targetPos), targetPos);
            push(std::move(out));
        }

        outs.swap(nextOuts);
        nextOuts.clear();
    }
    return outs;
}

size_t len(const Instructions& instructions) {
    size_t len{0};
    for (const auto& [segment, count] : instructions) {
        len += segment.size() * count;
    }
    return len;
}

void PruneSet(Set& set) {
    if (!set.empty()) {
        size_t minLength = ranges::min(set | views::transform(len));
        for (auto it2 = set.begin(); it2 != set.end();) {
            if (len(*it2) > minLength) {
                it2 = set.erase(it2);
            } else {
                ++it2;
            }
        }
    }
}

std::map<std::string, Set> cache;
const Set& ArrowToArrowCached(const std::string& input) {
    auto it = cache.find(input);
    if (it == cache.end()) {
        auto set = ArrowToArrowInner(input);
        PruneSet(set);
        it = cache.emplace(input, std::move(set)).first;
    }
    return it->second;
}

Set TransformInstructions(const Instructions& in) {
    Set outs{Instructions{}};
    Set nextOuts{};
    for (const auto& [inSegment, inCount] : in) {
        const Set& qwertyuiop = ArrowToArrowCached(inSegment);
        for (const Instructions& instrOption : qwertyuiop) {
            for (const Instructions& out : outs) {
                Instructions add{out};
                for (const auto& [optionArrows, optionCount] : instrOption) {
                    add[optionArrows] += inCount * optionCount;
                }
                nextOuts.emplace(add);
            }
        }
        outs.swap(nextOuts);
        nextOuts.clear();
    }
    return outs;
}

Set ApplyLayer(const Set& in) {
    Set out;
    for (const Instructions& instr : in) {
        out.merge(TransformInstructions(instr));
    }
    return out;
}

template <size_t layerCount>
size_t Complexity(std::string_view input) {
    Set layer        = NumToArrow(input);
    for (size_t i{0}; i < layerCount; ++i) {

        layer = ApplyLayer(layer);
        PruneSet(layer);

        std::println("{}[{}].size() == {}", input, i, layer.size());
    }

    const size_t num                = ParseNumber<size_t>(input.substr(0, input.size() - 1));
    const Instructions& minLenInstr = ranges::min(layer, std::less{}, len);
    const size_t minLen             = len(minLenInstr);
    std::println("\x1b[92m{}: {} * {} = {} | {}\x1b[0m", input, minLen, num, minLen * num, minLenInstr);
    return num * minLen;
}

} // namespace

void AocMain(std::string_view input) {
    input = test;

    // 161468
    // too high

    // 379A
    // <v<A>>^AvA^A<vA<AA>>^AAvA<^A>AAvA^A<vA>^AA<A>A<v<A>A>^AAAvA<^A>A
    // v<<A>>^AvA^Av<<A>>^AAv<A<A>>^AAvAA<^A>Av<A>^AA<A>Av<A<A>>^AAAvA<^A>A

    logger.solution("{}", ranges::accumulate(input | Split('\n'), size_t{}, std::plus{}, Complexity<25>));
}