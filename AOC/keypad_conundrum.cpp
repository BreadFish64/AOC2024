namespace {
[[maybe_unused]] constexpr std::string_view test = R"(029A
980A
179A
456A
379A
)";

// <Av<A>>^A
// v<<A>>^Av<A<A>>^AvA^AA
//
// v<<A>^A>A

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
    init['V'] = {1, 1};
    init['>'] = {1, 2};
    return init;
}();

std::string TransformDigits(std::string_view in) {
    std::string out;
    Pos2D robot{DIGIT_TO_POS['A']};
    for (const char target : in) {
        const Pos2D targetPos{DIGIT_TO_POS[target]};

        const auto MoveRight = [&] {
            while (robot.x() < targetPos.x()) {
                out.push_back('>');
                ++robot.x();
            }
        };
        const auto MoveLeft = [&] {
            while (robot.x() > targetPos.x()) {
                out.push_back('<');
                --robot.x();
            }
        };
        const auto MoveUp = [&] {
            while (robot.y() > targetPos.y()) {
                out.push_back('^');
                --robot.y();
            }
        };
        const auto MoveDown = [&] {
            while (robot.y() < targetPos.y()) {
                out.push_back('v');
                ++robot.y();
            }
        };

        if (robot.y() == 3 && targetPos.x() == 0) {
            MoveUp();
            MoveLeft();
        } else if (robot.x() == 0 && targetPos.y() == 3) {
            MoveRight();
            MoveDown();
        } else if (robot.y() > targetPos.y()) {
            if (robot.x() > targetPos.x()) {
                MoveLeft();
                MoveUp();
            } else {
                MoveUp();
                MoveRight();
            }
        } else {
            if (robot.x() > targetPos.x()) {
                MoveLeft();
                MoveDown();
            } else {
                MoveRight();
                MoveDown();
            }
        }
        out.push_back('A');
    }
    return out;
}

std::string TransformArrows(std::string_view in) {
    std::string out;
    Pos2D robot{ARROW_TO_POS['A']};
    for (const char target : in) {
        const Pos2D targetPos{ARROW_TO_POS[target]};

        auto MoveX = [&] {
            while (robot.x() < targetPos.x()) {
                out.push_back('>');
                ++robot.x();
            }
            while (robot.x() > targetPos.x()) {
                out.push_back('<');
                --robot.x();
            }
        };
        if (robot.y() > targetPos.y()) {
            MoveX();
            while (robot.y() > targetPos.y()) {
                out.push_back('^');
                --robot.y();
            }
        } else {
            while (robot.y() < targetPos.y()) {
                out.push_back('v');
                ++robot.y();
            }
            MoveX();
        }
        out.push_back('A');
    }
    return out;
}

size_t Complexity(std::string_view input) {
    const std::string first = TransformDigits(input);
    logger.info("{}", first);

    const std::string second = TransformArrows(first);
    logger.info("{}", second);

    const std::string third = TransformArrows(second);
    logger.info("{}", third);

    size_t complexity{third.length() * ParseNumber<size_t>(input.substr(0, input.size() - 1))};
    logger.info("{}", complexity);

    return complexity;
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

    logger.info("{}", TransformArrows("vA<A>^A"));
    logger.info("{}", TransformArrows("v<A>A^A"));
}