namespace {

constexpr uint8_t RIGHT = 0b01000000;
constexpr uint8_t DOWN  = 0b00010000;
constexpr uint8_t LEFT  = 0b00000100;
constexpr uint8_t UP    = 0b00000001;

constexpr uint8_t VISITED = 0b01010101;
constexpr uint8_t WALL    = 0b10101010;

mdarray<uint8_t, dextents<int32_t, 2>> MakePatrolGrid(const InputGrid<const char> inputGrid) {
    mdarray<uint8_t, dextents<int32_t, 2>> patrolGrid(inputGrid.extents());
    for (int32_t y{0}; y < patrolGrid.extent(0); ++y) {
        for (int32_t x{0}; x < patrolGrid.extent(1); ++x) {
            patrolGrid(y, x) = inputGrid(y, x) == '#' ? WALL : 0;
        }
    }
    return patrolGrid;
}

const std::array<Index2D, 128> DIRECTION_VECTOR = [] {
    std::array<Index2D, 128> init{};
    init[UP]    = {-1, 0};
    init[LEFT]  = {0, -1};
    init[DOWN]  = {1, 0};
    init[RIGHT] = {0, 1};
    return init;
}();

size_t PatrolCoverage(const mdspan<uint8_t, dextents<int32_t, 2>> grid, Index2D pos) {
    size_t coverage{0};
    uint8_t direction{1};
    while (pos[0] >= 0 && pos[0] < grid.extent(0) && pos[1] >= 0 && pos[1] < grid.extent(1)) {
        uint8_t& cell{grid(pos)};
        if (cell & WALL) {
            pos -= DIRECTION_VECTOR[direction];
            direction = std::rotr(direction, 2);
        } else {
            coverage += !(cell & VISITED);
            cell |= VISITED;
            pos += DIRECTION_VECTOR[direction];
        }
    }
    return coverage;
}

bool PatrolLoops(const mdspan<uint8_t, dextents<int32_t, 2>> grid, Index2D pos) {
    uint8_t direction{1};
    while (pos[0] >= 0 && pos[0] < grid.extent(0) && pos[1] >= 0 && pos[1] < grid.extent(1)) {
        uint8_t& cell{grid(pos)};
        const uint8_t oldCell{cell};
        cell = oldCell | direction;
        if (oldCell & direction) [[unlikely]] {
            return true;
        }
        if (cell & WALL) {
            pos -= DIRECTION_VECTOR[direction];
            direction = std::rotr(direction, 2);
        } else {
            pos += DIRECTION_VECTOR[direction];
        }
    }
    return false;
}

size_t PossibleLoops(const mdarray<uint8_t, dextents<int32_t, 2>>& masterGrid,
                     const mdspan<uint8_t, dextents<int32_t, 2>>& coverageGrid,
                     const Index2D initialPos) {
    size_t possibleLoops{0};
    for (int32_t y{0}; y < masterGrid.extent(0); ++y) {
        for (int32_t x{0}; x < masterGrid.extent(1); ++x) {
            if (!coverageGrid(y, x)) {
                continue;
            }
            mdarray gridCopy = masterGrid;
            gridCopy(y, x) |= WALL;
            possibleLoops += PatrolLoops(gridCopy, initialPos);
        }
    }
    return possibleLoops;
}

} // namespace

void AocMain(std::string_view input) {
    std::optional<StopWatch<std::micro>> setupStopwatch{"Setup"};
    mdspan inputGrid = ToGrid(input);
    size_t caretPos  = input.find('^');
    const Index2D initialPos{caretPos / inputGrid.stride(0), caretPos % inputGrid.stride(0)};
    const mdarray masterGrid = MakePatrolGrid(inputGrid);
    mdarray coverageGrid     = masterGrid;
    setupStopwatch.reset();

    logger.solution("PatrolCoverage: {}",
                    StopWatch<std::micro>::Run("PatrolCoverage", PatrolCoverage, coverageGrid, initialPos));
    logger.solution("PossibleLoops:  {}",
                    StopWatch<std::milli>::Run("PossibleLoops", PossibleLoops, masterGrid, coverageGrid, initialPos));
}