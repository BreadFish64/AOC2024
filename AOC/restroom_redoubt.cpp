namespace {

[[maybe_unused]] constexpr std::string_view test = R"(p=0,4 v=3,-3
p=6,3 v=-1,-3
p=10,3 v=-1,2
p=2,0 v=2,-1
p=0,0 v=1,3
p=3,0 v=-2,-2
p=7,6 v=-1,-3
p=3,0 v=-1,-2
p=9,3 v=2,3
p=7,3 v=-1,2
p=2,4 v=2,-3
p=9,5 v=-3,-3
)";

struct Robot {
    Index2D position;
    Index2D velocity;

    Robot(std::string_view line) {
        size_t numberStart = 2;
        size_t numberEnd   = line.find(',', numberStart);
        position[0]        = ParseNumber<int32_t>(line.substr(numberStart, numberEnd - numberStart));
        numberStart        = numberEnd + 1;
        numberEnd          = line.find(" v="sv, numberStart);
        position[1]        = ParseNumber<int32_t>(line.substr(numberStart, numberEnd - numberStart));
        numberStart        = numberEnd + 3;
        numberEnd          = line.find(',', numberStart);
        velocity[0]        = ParseNumber<int32_t>(line.substr(numberStart, numberEnd - numberStart));
        numberStart        = numberEnd + 1;
        velocity[1]        = ParseNumber<int32_t>(line.substr(numberStart));
    }
};

} // namespace

void AocMain(std::string_view input) {
    Index2D dims{101, 103};
#if false
    input = test;
    dims  = Index2D{11, 7};
#endif

    const int64_t lftMax = dims.x() / 2;
    const int64_t rgtMin = lftMax + dims.x() % 2;
    const int64_t botMax = dims.y() / 2;
    const int64_t topMin = botMax + dims.y() % 2;

    auto robots = input | Split('\n') | views::transform(Constructor<Robot>{}) | ranges::to_vector;
    std::array<size_t, 4> quadrantCounts{};
    for (const Robot& robot : robots) {
        Index2D endPosition = (robot.position + robot.velocity * 100);
        endPosition.x() %= dims.x();
        if (endPosition.x() < 0) endPosition.x() += dims.x();
        endPosition.y() %= dims.y();
        if (endPosition.y() < 0) endPosition.y() += dims.y();

        quadrantCounts[0] += (endPosition.x() >= rgtMin) && (endPosition.y() >= topMin);
        quadrantCounts[1] += (endPosition.x() >= rgtMin) && (endPosition.y() < botMax);
        quadrantCounts[2] += (endPosition.x() < lftMax) && (endPosition.y() >= topMin);
        quadrantCounts[3] += (endPosition.x() < lftMax) && (endPosition.y() < botMax);
    }
    logger.info("Quadrants: {}", quadrantCounts);
    logger.solution("Part 1: {}", ranges::accumulate(quadrantCounts, size_t{1}, std::multiplies{}));

    const auto printRobots = [&]() {
        std::string treeLine(static_cast<size_t>(dims.x()), ' ');
        treeLine.push_back('\n');
        std::string treeStorage;
        for (int64_t y{0}; y < dims.y(); ++y) {
            treeStorage += treeLine;
        }
        mdspan treeGrid(treeStorage.data(), layout_left_padded<dynamic_extent>::mapping<dextents<int32_t, 2>>(
                                                dextents<int32_t, 2>(dims.x(), dims.y()), dims.x() + 1));
        for (const Robot& robot : robots) {
            char& cell{treeGrid(robot.position)};
            if (cell == ' ') {
                cell = '1';
            } else {
                cell += 1;
            }
        }
        logger.info("-----\n{}-----", std::move(treeStorage));
        logger.flush();
    };
    printRobots();
    const auto advance = [&](int64_t steps) {
        for (Robot& robot : robots) {
            robot.position = (robot.position + robot.velocity * steps);
            robot.position.x() %= dims.x();
            if (robot.position.x() < 0) robot.position.x() += dims.x();
            robot.position.y() %= dims.y();
            if (robot.position.y() < 0) robot.position.y() += dims.y();
        }
    };

    std::array<int64_t, 2> mods{128 - 27, 178 - 75};
    std::array<int64_t, 2> remainders{27, 75};
    int64_t minStep = ChineseRemainderTheorem<int64_t>(mods, remainders) - 2;
    advance(minStep);
    for (int64_t i{minStep};; ++i) {
        logger.info("i = {}", i);
        printRobots();
        std::cin.ignore();
        advance(1);
    }
}