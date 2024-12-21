namespace {

constexpr uint8_t RIGHT = 0;
constexpr uint8_t DOWN  = 1;
constexpr uint8_t LEFT  = 2;
constexpr uint8_t UP    = 3;

const std::array<Vec2D, 4> DIRECTION_VECTOR = [] {
    std::array<Vec2D, 4> init{};
    init[UP]    = {-1, 0};
    init[LEFT]  = {0, -1};
    init[DOWN]  = {1, 0};
    init[RIGHT] = {0, 1};
    return init;
}();

struct Maze {
    using Score = int32_t;

    struct MazeCell {
        std::array<Score, 4> forward{};
        Score back{std::numeric_limits<Score>::max()};
    };

    mdarray<MazeCell, dextents<int32_t, 2>> grid;
    Pos2D startPos;
    Pos2D endPos;

    Maze(std::string_view inputString) {
        const mdspan inputMaze = ToGrid(inputString);
        const size_t startIndex{inputString.find('S')};
        const size_t endIndex{inputString.find('E')};

        startPos = {static_cast<int32_t>(startIndex / inputMaze.stride(0)), static_cast<int32_t>(startIndex % inputMaze.stride(0))};
        endPos   = {static_cast<int32_t>(endIndex / inputMaze.stride(0)), static_cast<int32_t>(endIndex % inputMaze.stride(0))};
        grid     = {inputMaze.extents()};

        for (int32_t y{0}; y < inputMaze.extent(0); ++y) {
            for (int32_t x{0}; x < inputMaze.extent(1); ++x) {
                MazeCell& cell{grid(y, x)};
                cell.forward.fill((inputMaze(y, x) == '#') ? -1 : std::numeric_limits<Score>::max());
            }
        }
    }

    Score explore(const Pos2D pos, const Score currentScore, const uint8_t direction) {
        MazeCell& cell{grid.to_mdspan()(pos)};
        if (cell.forward[direction] < currentScore) {
            return std::numeric_limits<Score>::max();
        }
        cell.forward[direction] = currentScore;
        if (pos == endPos) {
            cell.back = std::min(currentScore, cell.back);
            return currentScore;
        }
        Score ret = explore(pos + DIRECTION_VECTOR[direction], currentScore + 1, direction);
        ret       = std::min(
            ret, explore(pos + DIRECTION_VECTOR[(direction + 1) & 0b11], currentScore + 1001, (direction + 1) & 0b11));
        ret = std::min(
            ret, explore(pos + DIRECTION_VECTOR[(direction - 1) & 0b11], currentScore + 1001, (direction - 1) & 0b11));
        cell.back = std::min(ret, cell.back);
        return ret;
    }

    void explore() { explore(startPos, 0, RIGHT); }

    Score getEndScore() const noexcept { return grid.to_mdspan()(endPos).back; }

    size_t countBestPathCells() const noexcept {
        const Score lowestScore{getEndScore()};
        size_t count{0};
        for (int32_t y{0}; y < grid.extent(0); ++y) {
            for (int32_t x{0}; x < grid.extent(1); ++x) {
                count += grid(y, x).back == lowestScore;
            }
        }
        return count;
    }
};

} // namespace

void AocMain(std::string_view input) {
    Maze maze(input);
    maze.explore();
    logger.solution("{}", maze.getEndScore());
    logger.solution("{}", maze.countBestPathCells());
}