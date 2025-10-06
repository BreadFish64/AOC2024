#include <print>

namespace {
[[maybe_unused]] constexpr std::string_view test = R"(029A
980A
179A
456A
379A
)";

using Instructions = std::unordered_map<std::string, size_t>;

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

using Set = std::set<std::string>;

Set NumToArrow(std::string_view input) {
    Set outs{""};
    Set nextOuts{};
    Pos2D robotPos{DIGIT_TO_POS['A']};
    for (const char digit : input) {
        const Pos2D targetPos{DIGIT_TO_POS[digit]};
        std::string out;

        auto push = [&](std::string&& str) {
            std::string toAdd{std::move(str)};
            toAdd.push_back('A');
            for (const std::string& in : outs) {
                nextOuts.emplace(in + toAdd);
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

Set ArrowToArrow(std::string_view input) {
    Set outs{""};
    Set nextOuts{};
    Pos2D robotPos{ARROW_TO_POS['A']};
    for (const char digit : input) {
        const Pos2D targetPos{ARROW_TO_POS[digit]};
        std::string out;

        auto push = [&](std::string&& str) {
            std::string toAdd{std::move(str)};
            toAdd.push_back('A');
            for (const std::string& in : outs) {
                nextOuts.emplace(in + toAdd);
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

Set ApplyLayer(const Set& in) {
    Set out;
    for (std::string_view str : in) {
        out.merge(ArrowToArrow(str));
    }
    return out;
}

size_t Complexity(std::string_view input) {
    const Set first = NumToArrow(input);
    std::println("{}: {}", input, first);
    const auto second = ApplyLayer(first);
    std::println("{}: {}", input, second);
    const auto third = ApplyLayer(second);
    //std::println("{}: {}", input, third);

    size_t num = ParseNumber<size_t>(input.substr(0, input.size() - 1));
    const std::string& minMoves = ranges::min(third, std::less{}, &std::string::size);
    std::println("\x1b[92m{}: {} * {} = {} | {}\x1b[0m", input, minMoves.size(), num, num * minMoves.size(), minMoves);
    return minMoves.size() * num;
}

} // namespace

void AocMain(std::string_view input) {
    //input = test;

    // 161468
    // too high

    // 379A
    // <v<A>>^AvA^A<vA<AA>>^AAvA<^A>AAvA^A<vA>^AA<A>A<v<A>A>^AAAvA<^A>A
    // v<<A>>^AvA^Av<<A>>^AAv<A<A>>^AAvAA<^A>Av<A>^AA<A>Av<A<A>>^AAAvA<^A>A

    logger.solution("{}", ranges::accumulate(input | Split('\n'), size_t{}, std::plus{}, Complexity));
}