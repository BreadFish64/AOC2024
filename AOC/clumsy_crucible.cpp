namespace {
using Coord = Eigen::Array<u8, 2, 1>;
using Loss  = u64;
struct CumulativeLoss {
    std::array<std::array<Loss, 4>, 10> losses;
    Loss minimumLoss;
    CumulativeLoss() {
        for (auto& straightCount : losses) {
            for (Loss& direction : straightCount) {
                direction = std::numeric_limits<Loss>::max();
            }
        }
        minimumLoss = std::numeric_limits<Loss>::max();
    }
};
using LossGrid           = std::mdspan<const u8, std::dextents<u32, 2>>;
using CumulativeLossGrid = std::mdspan<CumulativeLoss, std::dextents<u32, 2>>;
constexpr u8 RIGHT = (1 << 0), DOWN = (1 << 2), LEFT = (1 << 4), UP = (1 << 6);
constexpr u8 DIRECTION_MASK = RIGHT | DOWN | LEFT | UP;

constexpr void AssertIsDirection(u8 direction) {
    Assume(std::has_single_bit(direction) && (direction & DIRECTION_MASK));
}
constexpr u8 Right(u8 direction) {
    AssertIsDirection(direction);
    return std::rotl(direction, 2);
}
constexpr u8 Left(u8 direction) {
    AssertIsDirection(direction);
    return std::rotr(direction, 2);
}
constexpr size_t DirectionIndex(u8 direction) {
    return std::countl_zero(direction) / 2_sz;
}
const std::array<Coord, 4> DIRECTION_OFFSETS{{
    Coord{0, 1},
    Coord{1, 0},
    Coord{0, -1},
    Coord{-1, 0},
}};
const std::array<Coord, 128> DIRECTION_OFFSETS2 = [] {
    std::array<Coord, 128> tmp{};
    tmp[RIGHT] = DIRECTION_OFFSETS[0];
    tmp[DOWN]  = DIRECTION_OFFSETS[1];
    tmp[LEFT]  = DIRECTION_OFFSETS[2];
    tmp[UP]    = DIRECTION_OFFSETS[3];
    return tmp;
}();
Coord DirectionOffset(u8 direction) {
    AssertIsDirection(direction);
    return DIRECTION_OFFSETS2[direction];
}

auto MakeGrid(std::string_view input) {
    const u32 width  = input.find('\n');
    const u32 stride = width + 1;
    const u32 height = input.size() / stride;
    std::mdspan<const char, std::dextents<u32, 2>> input2d{input.data(), height, stride};

    std::vector<u8> lossGridStorage(input2d.size());
    std::mdspan<u8, std::dextents<u32, 2>> lossMap{lossGridStorage.data(), height, width};
    for (const u32 y : views::iota(0u, height)) {
        for (const u32 x : views::iota(0u, width)) {
            lossMap(y, x) = input2d(y, x) - '0';
        }
    }
    return std::make_tuple(std::move(lossGridStorage), LossGrid{lossMap});
}

template <u32 MINIMUM_STRAIGHT, u32 MAXIMUM_STRAIGHT>
void OptimizePath(const LossGrid& lossGrid, const CumulativeLossGrid& cumulativeLossGrid, const Loss& currentLowestLoss,
                  Loss cumulativeLoss, Coord coord, u8 direction, u8 straightCount) {
    if (coord[0] >= cumulativeLossGrid.extent(0) || coord[1] >= cumulativeLossGrid.extent(1) ||
        cumulativeLoss >= currentLowestLoss) {
        return;
    }
    cumulativeLoss += lossGrid[ToSpan(coord)];
    CumulativeLoss& minimumLoss = cumulativeLossGrid[ToSpan(coord)];
    Loss& minimumLossForState   = minimumLoss.losses[straightCount][DirectionIndex(direction)];
    if (cumulativeLoss >= minimumLossForState) {
        return;
    }
    minimumLossForState = cumulativeLoss;
    if (straightCount + 1 >= MINIMUM_STRAIGHT) {
        minimumLoss.minimumLoss = std::min(minimumLoss.minimumLoss, cumulativeLoss);
        const u8 left           = Left(direction);
        OptimizePath<MINIMUM_STRAIGHT, MAXIMUM_STRAIGHT>(lossGrid, cumulativeLossGrid, currentLowestLoss,
                                                         cumulativeLoss, coord + DirectionOffset(left), left, 0);
        const u8 right = Right(direction);
        OptimizePath<MINIMUM_STRAIGHT, MAXIMUM_STRAIGHT>(lossGrid, cumulativeLossGrid, currentLowestLoss,
                                                         cumulativeLoss, coord + DirectionOffset(right), right, 0);
    }
    if (straightCount < (MAXIMUM_STRAIGHT - 1)) {
        OptimizePath<MINIMUM_STRAIGHT, MAXIMUM_STRAIGHT>(lossGrid, cumulativeLossGrid, currentLowestLoss,
                                                         cumulativeLoss, coord + DirectionOffset(direction), direction,
                                                         straightCount + 1);
    }
}

template <u32 MINIMUM_STRAIGHT, u32 MAXIMUM_STRAIGHT>
u64 EstimateUpperBound(LossGrid lossGrid) {
    Assume(lossGrid.extent(0) == lossGrid.extent(1));
    u64 upperBound = 0;
    Coord coord{0, 0};
    u32 distanceFromEdge{};
    do {
        for (const u32 rep : views::iota(0u, MINIMUM_STRAIGHT)) {
            coord += DirectionOffset(RIGHT);
            upperBound += lossGrid[ToSpan(coord)];
        }
        for (const u32 rep : views::iota(0u, MINIMUM_STRAIGHT)) {
            coord += DirectionOffset(DOWN);
            upperBound += lossGrid[ToSpan(coord)];
        }
        distanceFromEdge = lossGrid.extent(0) - 1 - coord[0];
    } while (distanceFromEdge > MAXIMUM_STRAIGHT);
    for (const u32 rep : views::iota(0u, distanceFromEdge)) {
        coord += DirectionOffset(RIGHT);
        upperBound += lossGrid[ToSpan(coord)];
    }
    for (const u32 rep : views::iota(0u, distanceFromEdge)) {
        coord += DirectionOffset(DOWN);
        upperBound += lossGrid[ToSpan(coord)];
    }
    return upperBound;
}

template <u32 MINIMUM_STRAIGHT, u32 MAXIMUM_STRAIGHT>
u64 Solve(const LossGrid& lossGrid) {
    std::vector<CumulativeLoss> cumulativeLossGridStorage(lossGrid.size());
    CumulativeLossGrid cumulativeLossGrid{cumulativeLossGridStorage.data(), lossGrid.extents()};
    Loss& currentLowestLoss = cumulativeLossGridStorage.back().minimumLoss;
    currentLowestLoss =
        EstimateUpperBound<MINIMUM_STRAIGHT, MAXIMUM_STRAIGHT>(lossGrid);
    OptimizePath<MINIMUM_STRAIGHT, MAXIMUM_STRAIGHT>(lossGrid, cumulativeLossGrid, currentLowestLoss, 0, Coord{0, 1},
                                                     RIGHT, 0);
    OptimizePath<MINIMUM_STRAIGHT, MAXIMUM_STRAIGHT>(lossGrid, cumulativeLossGrid, currentLowestLoss, 0, Coord{1, 0},
                                                     DOWN, 0);
    return currentLowestLoss;
}

} // namespace

void AocMain(std::string_view input) {
    auto [lossGridStorage, lossGrid] = StopWatch<std::micro>::Run("Setup grid", MakeGrid, input);
    logger.solution("Part 1: {}", StopWatch<std::ratio<1, 1>>::Run("Part 1", Solve<0, 3>, lossGrid));
    logger.solution("Part 2: {}", StopWatch<std::ratio<1, 1>>::Run("Part 2", Solve<4, 10>, lossGrid));
}