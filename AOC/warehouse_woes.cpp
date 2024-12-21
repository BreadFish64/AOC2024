namespace {

template <char box>
size_t GPSSum(const InputGrid<const char> warehouse) {
    size_t sum{0};
    for (int32_t y{1}; y < warehouse.extent(0) - 1; ++y) {
        for (int32_t x{1}; x < warehouse.extent(1) - 1; ++x) {
            sum += (warehouse(y, x) == box) ? (100_size_t * y + x) : 0;
        }
    }
    return sum;
}

Index2D MoveRobotSimple(const InputGrid<char> warehouse, const Index2D direction, const Index2D robot) {
    Index2D probe(robot + direction);
    while (warehouse(probe) != '.') {
        if (warehouse(probe) == '#') {
            return robot;
        }
        probe += direction;
    }
    while (!(probe == robot).all()) {
        Index2D nextProbe{probe - direction};
        std::swap(warehouse(probe), warehouse(nextProbe));
        probe = nextProbe;
    }
    return robot + direction;
}

template <bool execute>
bool MoveRobotWideImpl(const InputGrid<std::conditional_t<execute, char, const char>> warehouse,
                           const Index2D direction, const Index2D position) {
    const char cell{warehouse(position)};
    if (cell == '#') {
        if constexpr (execute) {
            std::unreachable();
        } else {
            return false;
        }
    }
    if (cell == '.') return true;
    Index2D left;
    Index2D right;
    if (cell == '[') {
        left  = position;
        right = position + Index2D{0, 1};
    } else if (cell == ']') {
        left  = position + Index2D{0, -1};
        right = position;
    } else {
        std::unreachable();
    }
    const bool canMove = MoveRobotWideImpl<execute>(warehouse, direction, left + direction) &&
                         MoveRobotWideImpl<execute>(warehouse, direction, right + direction);
    if constexpr (execute) {
        if (!canMove) std::unreachable();
        std::swap(warehouse(left), warehouse(Index2D{left + direction}));
        std::swap(warehouse(right), warehouse(Index2D{right + direction}));
    }
    return canMove;
}

Index2D MoveRobotWide(const InputGrid<char> warehouse, const Index2D direction, const Index2D robot) {
    const Index2D nextRobotPos{robot + direction};
    if (!MoveRobotWideImpl<false>(warehouse, direction, nextRobotPos)) {
        return robot;
    }
    MoveRobotWideImpl<true>(warehouse, direction, nextRobotPos);
    std::swap(warehouse(robot), warehouse(nextRobotPos));
    return nextRobotPos;
}

std::string WidenWarehouse(const std::string_view input) {
    std::string warehouse;
    warehouse.reserve(input.size() * 2);
    for (const char cell : input) {
        switch (cell) {
            case '#':
            case '.': warehouse.insert(warehouse.end(), 2, cell); break;
            case 'O': warehouse += "[]"sv; break;
            case '@': warehouse += "@."sv; break;
            case '\n': warehouse += '\n'; break;
            default: std::unreachable();
        }
    }
    return warehouse;
}

template <bool wide, bool print = false>
size_t Simulate(const std::string_view input) {
    const size_t inputSplit{input.find("\n\n"sv)};
    const std::string_view inputWarehouse{input.substr(0, inputSplit + 1)};
    const std::string_view moves = input.substr(inputSplit + 2);

    std::string state{wide ? WidenWarehouse(inputWarehouse) : inputWarehouse};
    const mdspan warehouse{ToGrid(state)};

    const size_t initialRobotIndex = state.find('@');
    Index2D robot{initialRobotIndex / warehouse.stride(0), initialRobotIndex % warehouse.stride(0)};

    for (const char move : moves) {
        switch (move) {
            case '>': robot = MoveRobotSimple(warehouse, {0, 1}, robot); break;
            case 'v':
                robot =
                    wide ? MoveRobotWide(warehouse, {1, 0}, robot) : MoveRobotSimple(warehouse, {1, 0}, robot);
                break;
            case '<': robot = MoveRobotSimple(warehouse, {0, -1}, robot); break;
            case '^':
                robot =
                    wide ? MoveRobotWide(warehouse, {-1, 0}, robot) : MoveRobotSimple(warehouse, {-1, 0}, robot);
                break;
            default:
                if constexpr (print) logger.info("Skipping invalid move: {:02X}", move);
                break;
        }
        if constexpr (print) logger.info("\n{}\n{}", move, state);
    }
    return GPSSum<(wide ? '[' : 'O')>(warehouse);
}

} // namespace

void AocMain(std::string_view input) {
    logger.solution("Narrow: {}", StopWatch<std::micro>::Run("Simulate Narrow", Simulate<false>, input));
    logger.solution("Wide:   {}", StopWatch<std::micro>::Run("Simulate Wide  ", Simulate<true>, input));
}