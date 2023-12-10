namespace {
using PipeMap      = std::mdspan<u8, std::dextents<size_t, 2>>;
using Coord        = Eigen::Array<size_t, 2, 1>;
constexpr u8 RIGHT = (1 << 0), DOWN = (1 << 2), LEFT = (1 << 4), UP = (1 << 6);
constexpr u8 DIRECTION_MASK = RIGHT | DOWN | LEFT | UP;
constexpr u8 CHECKED_MASK   = static_cast<u8>(~DIRECTION_MASK);
constexpr u8 IS_PIPE        = (1 << 1);
constexpr u8 IS_FLOODED     = (1 << 7);
constexpr std::array<u8, 4> DIRECTIONS{{RIGHT, DOWN, LEFT, UP}};

constexpr void AssertIsDirection(u8 direction) {
    Assume(std::has_single_bit(direction) && (direction & DIRECTION_MASK));
}
constexpr u8 Opposite(u8 direction) {
    AssertIsDirection(direction);
    return std::rotr(direction, 4);
}
constexpr u8 Right(u8 direction) {
    AssertIsDirection(direction);
    return std::rotl(direction, 2);
}
constexpr u8 Left(u8 direction) {
    AssertIsDirection(direction);
    return std::rotr(direction, 2);
}
const std::array<Coord, 4> DIRECTION_OFFSETS{{
    Coord{0, 1},
    Coord{1, 0},
    Coord{0, -1},
    Coord{-1, 0},
}};
Coord DirectionOffset(u8 direction) {
    AssertIsDirection(direction);
    return DIRECTION_OFFSETS[std::countr_zero(direction) >> 1];
}

s64 Flood(const PipeMap pipeMap, const Coord position, const u8 entryDirection) {
    if (position[0] >= pipeMap.extent(0) || position[1] >= pipeMap.extent(1)) return 0;
    {
        u8& cellRef = pipeMap[ToSpan(position)];
        u8 cell     = cellRef;
        if (cell & CHECKED_MASK) return 0;
        cellRef = cell | IS_FLOODED;
    }
    return ranges::accumulate(DIRECTIONS, s64{1}, std::plus{}, [&](const u8 exitDirection) -> s64 {
        if (exitDirection == entryDirection) return 0;
        return Flood(pipeMap, position + DirectionOffset(exitDirection), Opposite(exitDirection));
    });
}

void TraversePipe(const PipeMap pipeMap, const Coord startPos,
                  std::invocable<const Coord&, u8&, u8, u8> auto&& callback) {
    Coord position{startPos};
    // Pick one direction to traverse
    const u8 startingPipeConnections = pipeMap[ToSpan(position)] & DIRECTION_MASK;
    u8 entryDirection = startingPipeConnections ^ (1 << std::countr_zero<u8>(startingPipeConnections & DIRECTION_MASK));
    do {
        u8& pipe           = pipeMap[ToSpan(position)];
        const u8 direction = entryDirection ^ (pipe & DIRECTION_MASK);
        callback(position, pipe, entryDirection, direction);
        entryDirection = Opposite(direction);
        position += DirectionOffset(direction);
    } while (!ranges::equal(startPos, position));
}

void Solve(const PipeMap pipeMap, const Coord startPos) {
    s64 pipeCount{};
    {
        StopWatch<std::micro> part1Watch{"Marking Pipes"};
        TraversePipe(pipeMap, startPos,
                     [&]([[maybe_unused]] const Coord& position, u8& pipe, [[maybe_unused]] u8 entryDirection,
                         [[maybe_unused]] u8 exitDirection) {
                         ++pipeCount;
                         pipe |= IS_PIPE;
                     });
    }
    logger.solution("Part 1: {}", pipeCount / 2);

    s64 floodedCount{};
    {
        StopWatch<std::micro> part2Watch{"Flooding"};
        TraversePipe(pipeMap, startPos,
                     [&](const Coord& position, [[maybe_unused]] u8& pipe, u8 entryDirection, u8 exitDirection) {
                         for (u8 floodDirection : {Right(entryDirection), Left(exitDirection)}) {
                             floodedCount +=
                                 Flood(pipeMap, position + DirectionOffset(floodDirection), Opposite(floodDirection));
                         }
                     });
    }
    logger.solution("Pipes:    {}", pipeCount);
    logger.solution("Flooded:  {}", floodedCount);
    logger.solution("Unmarked: {}", pipeMap.size() - floodedCount - pipeCount);
}

auto Parse(std::string_view input) {
    const size_t width  = input.find('\n');
    const size_t stride = width + 1;
    const size_t height = input.size() / stride;
    std::mdspan<const char, std::dextents<size_t, 2>> inputGrid{input.data(), height, stride};
    std::vector<u8> pipeStorage(height * width);
    PipeMap pipeMap{pipeStorage.data(), height, width};
    Coord startPos{};
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            pipeMap(y, x) = [&]() -> u8 {
                switch (inputGrid(y, x)) {
                    case '|': return DOWN | UP;
                    case '-': return RIGHT | LEFT;
                    case 'L': return RIGHT | UP;
                    case 'J': return LEFT | UP;
                    case '7': return DOWN | LEFT;
                    case 'F': return RIGHT | DOWN;
                    case '.': return u8{};
                    case '\n': return u8{};
                    case 'S': startPos = {y, x}; return u8{};
                    default: assert(false); return u8{};
                }
            }();
        }
    }
    // Figure out which connections the starting pipe has
    for (const u8 direction : DIRECTIONS) {
        if (pipeMap[ToSpan(Coord{startPos + DirectionOffset(direction)})] & Opposite(direction)) {
            pipeMap[ToSpan(startPos)] |= direction;
        }
    }
    return std::make_tuple(std::move(pipeStorage), pipeMap, startPos);
}

std::vector<u32> RenderVisual(PipeMap pipeMap) {
    std::vector<u32> imageStorage(pipeMap.size() * 4 * 4);
    std::mdspan<u32, std::dextents<size_t, 2>> pipeSprites(imageStorage.data(), pipeMap.extent(0) * 4,
                                                           pipeMap.extent(1) * 4);
    using SpriteSlice =
        std::strided_slice<size_t, std::integral_constant<size_t, 4>, std::integral_constant<size_t, 1>>;
    constexpr auto ColorSprite = [](const auto& sprite, u32 color) {
        for (size_t y = 0; y < sprite.extent(0); ++y) {
            for (size_t x = 0; x < sprite.extent(1); ++x) {
                sprite(y, x) = color;
            }
        }
    };
    for (size_t y = 0; y < pipeMap.extent(0); ++y) {
        for (size_t x = 0; x < pipeMap.extent(1); ++x) {
            const auto sprite = std::submdspan(pipeSprites, SpriteSlice{y * 4}, SpriteSlice{x * 4});
            const u8 cell     = pipeMap(y, x);
            if (cell & IS_PIPE) {
                ColorSprite(sprite, 0xFF00FF00U);
            } else if (cell & IS_FLOODED) {
                ColorSprite(sprite, 0xFFFF0000U);
            } else {
                ColorSprite(sprite, 0xFF0000FFU);
            }
            if (cell & DIRECTION_MASK) {
                if (cell & UP) {
                    sprite(0, 1) = 0xFFFFFFFFU;
                    sprite(0, 2) = 0xFFFFFFFFU;
                }
                if (cell & LEFT) {
                    sprite(1, 0) = 0xFFFFFFFFU;
                    sprite(2, 0) = 0xFFFFFFFFU;
                }
                sprite(1, 1) = 0xFFFFFFFFU;
                sprite(1, 2) = 0xFFFFFFFFU;
                sprite(2, 1) = 0xFFFFFFFFU;
                sprite(2, 2) = 0xFFFFFFFFU;
                if (cell & RIGHT) {
                    sprite(1, 3) = 0xFFFFFFFFU;
                    sprite(2, 3) = 0xFFFFFFFFU;
                }
                if (cell & DOWN) {
                    sprite(3, 1) = 0xFFFFFFFFU;
                    sprite(3, 2) = 0xFFFFFFFFU;
                }
            }
        }
    }
    return imageStorage;
}

} // namespace

void AocMain(std::string_view input) {
    auto [pipeStorage, pipeMap, startPos] = StopWatch<std::micro>::Run("Parse", Parse, input);
    Solve(pipeMap, startPos);
    const std::vector<u32> visual{StopWatch<std::micro>::Run("RenderVisual", RenderVisual, pipeMap)};
    lodepng::encode("pipes.png", reinterpret_cast<const unsigned char*>(visual.data()), pipeMap.extent(1) * 4,
                    pipeMap.extent(0) * 4);
}