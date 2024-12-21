namespace {
[[maybe_unused]] constexpr std::string_view test = R"(5,4
4,2
4,5
3,0
2,1
6,3
2,4
1,5
0,6
3,3
2,6
5,1
1,2
5,5
2,5
6,5
1,4
0,4
6,4
1,1
6,1
1,0
0,5
1,6
2,0
)";

std::pmr::unsynchronized_pool_resource pool;

std::vector<Pos2D> Parse(const std::string_view input) {
    return input | Split('\n') | views::transform([](std::string_view line) {
               const size_t comma = line.find(',');
               return Pos2D{ParseNumber<int32_t>(line.substr(comma + 1)), ParseNumber<int32_t>(line.substr(0, comma))};
           }) |
           ranges::to_vector;
}

int32_t BFS(const std::span<const Pos2D> bytes, const int32_t gridDim) {
    mdarray<int32_t, dextents<int32_t, 2>, layout_right, std::pmr::vector<int32_t>> memorySpace{
        dextents<int32_t, 2>{gridDim, gridDim}, std::pmr::polymorphic_allocator{&pool}};
    for (int32_t y{0}; y < memorySpace.extent(0); ++y) {
        for (int32_t x{0}; x < memorySpace.extent(1); ++x) {
            memorySpace(y, x) = std::numeric_limits<int32_t>::max();
        }
    }
    for (Pos2D byte : bytes) {
        memorySpace(byte.y(), byte.x()) = -1;
    }
    const int32_t& result{memorySpace(gridDim - 1, gridDim - 1)};

    std::pmr::vector<Pos2D> edges{std::pmr::polymorphic_allocator{&pool}};
    edges.push_back({0, 0});
    std::pmr::vector<Pos2D> nextEdges{std::pmr::polymorphic_allocator{&pool}};
    int32_t distance{1};
    while (!edges.empty() && (result == std::numeric_limits<int32_t>::max())) {
        for (Pos2D edge : edges) {
            for (Vec2D offset : {Vec2D{0, 1}, Vec2D{1, 0}, Vec2D{-1, 0}, Vec2D{0, -1}}) {
                Pos2D nextEdge{edge + offset};
                if (InBounds(memorySpace.extents(), nextEdge) &&
                    memorySpace(nextEdge.y(), nextEdge.x()) == std::numeric_limits<int32_t>::max()) {
                    memorySpace(nextEdge.y(), nextEdge.x()) = distance;
                    nextEdges.emplace_back(nextEdge);
                }
            }
        }
        ++distance;
        edges.swap(nextEdges);
        nextEdges.clear();
    }

    return result;
}

int32_t Part1(const std::span<const Pos2D> bytes, const int32_t gridDim, const size_t minFallenCount) {
    return BFS(std::span{bytes}.first(minFallenCount), gridDim);
}

std::pair<int32_t, int32_t> Part2(const std::span<const Pos2D> bytes, const int32_t gridDim,
                                  const size_t minFallenCount) {
    const size_t cutoff = *ranges::lower_bound(
        views::closed_iota(minFallenCount, bytes.size()), std::numeric_limits<int32_t>::max(), std::less{},
        [&bytes, gridDim](const size_t fallenCount) { return BFS(std::span{bytes}.first(fallenCount), gridDim); });
    return {bytes[cutoff - 1].x(), bytes[cutoff - 1].y()};
}

} // namespace

void AocMain(std::string_view input) {
#if false
    constexpr int32_t gridDim       = 7;
    constexpr size_t minFallenCount = 12;
    input                           = test;
#else
    constexpr int32_t gridDim       = 71;
    constexpr size_t minFallenCount = 1024;
#endif

    std::vector<Pos2D> bytes = StopWatch<std::micro>::Run("Parse", Parse, input);

    logger.solution("Part 1: {}", StopWatch<std::micro>::Run("Part 1", Part1, bytes, gridDim, minFallenCount));
    logger.solution("Part 2: {}", StopWatch<std::micro>::Run("Part 2", Part2, bytes, gridDim, minFallenCount));
}