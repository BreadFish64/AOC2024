namespace {

struct Course {
    mdarray<int32_t, dextents<int32_t, 2>> courseStorage;
    Pos2D startPos{};
    Pos2D endPos{};

    Course(std::string_view input) {
        const mdspan inputGrid = ToGrid(input);

        const size_t startIdx = input.find('S');
        startPos              = {static_cast<int32_t>(startIdx / inputGrid.stride(0)),
                                 static_cast<int32_t>(startIdx % inputGrid.stride(0))};
        const size_t endIdx   = input.find('E');
        endPos                = {static_cast<int32_t>(endIdx / inputGrid.stride(0)),
                                 static_cast<int32_t>(endIdx % inputGrid.stride(0))};

        courseStorage = {inputGrid.extents()};
        const mdspan<int32_t, dextents<int32_t, 2>> course{courseStorage};

        for (int32_t y{0}; y < course.extent(0); ++y) {
            for (int32_t x{0}; x < course.extent(1); ++x) {
                course(y, x) = (inputGrid(y, x) == '#') ? -1 : std::numeric_limits<int32_t>::max();
            }
        }
    }

    void pathFind() {
        const mdspan<int32_t, dextents<int32_t, 2>> course{courseStorage};
        int32_t distance{0};
        Pos2D pos{startPos};
        while (pos != endPos) {
            course(pos) = distance;
            Pos2D nextPos{pos};
            for (Vec2D offset : {Vec2D{-1, 0}, Vec2D{0, -1}, Vec2D{0, 1}, Vec2D{1, 0}}) {
                Pos2D offsetPos         = pos + offset;
                const int32_t maybeNext = course(offsetPos);
                if (maybeNext > distance) {
                    nextPos = offsetPos;
                }
            }
            pos = nextPos;
            ++distance;
        }
        course(pos) = distance;
    }

    size_t cheatCount(int32_t minSkip, const int32_t radius) const {
        const mdspan<const int32_t, dextents<int32_t, 2>> course{courseStorage};
        size_t count{0};
        for (int32_t y1{1}; y1 < course.extent(0) - 1; ++y1) {
            for (int32_t x1{1}; x1 < course.extent(1) - 1; ++x1) {
                const int32_t start = course(y1, x1);
                if (start < 0) {
                    continue;
                }
                for (int32_t y2 = std::max(0, y1 - radius); y2 <= std::min(y1 + radius, course.extent(0) - 1); ++y2) {
                    const int32_t yDist   = std::abs(y2 - y1);
                    const int32_t xRadius = radius - yDist;
                    for (int32_t x2 = std::max(0, x1 - xRadius); x2 <= std::min(x1 + xRadius, course.extent(1) - 1);
                         ++x2) {
                        const int32_t taxicab = yDist + std::abs(x2 - x1);
                        const int32_t saved   = (course(y2, x2) - start) - taxicab;
                        if (saved >= minSkip) {
                            ++count;
                        }
                    }
                }
            }
        }
        return count;
    }
};

} // namespace

void AocMain(std::string_view input) {
    Course course = StopWatch<std::micro>::Run("Parse", Constructor<Course>{}, input);
    StopWatch<std::micro>::Run("Pathfind", &Course::pathFind, course);
    logger.solution("Part 1: {}", StopWatch<std::micro>::Run("Part 1", &Course::cheatCount, course, 100, 2));
    logger.solution("Part 2: {}", StopWatch<std::micro>::Run("Part 2", &Course::cheatCount, course, 100, 20));
}