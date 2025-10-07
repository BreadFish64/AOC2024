namespace {

template <int32_t yInc, int32_t xInc>
bool CheckMatch(const InputGrid<const char> grid, int32_t y, int32_t x) {
    const auto CheckLetters = [&](auto... letter) {
        bool pass{true};
        ((pass &= (grid[y, x] == letter), y += yInc, x += xInc), ...);
        return pass;
    };
    return CheckLetters('X', 'M', 'A', 'S');
}

size_t Part1(const InputGrid<const char> grid) {
    size_t count{0};
    for (int32_t y = 0; y < grid.extent(1); ++y) {
        for (int32_t x = 0; x < grid.extent(0) - 3; ++x) {
            count += CheckMatch<0, 1>(grid, y, x) ? 1 : 0;
            count += CheckMatch<0, -1>(grid, y, x + 3) ? 1 : 0;
        }
    }
    for (int32_t y = 0; y < grid.extent(1) - 3; ++y) {
        for (int32_t x = 0; x < grid.extent(0); ++x) {
            count += CheckMatch<1, 0>(grid, y, x) ? 1 : 0;
            count += CheckMatch<-1, 0>(grid, y + 3, x) ? 1 : 0;
        }
    }
    for (int32_t y = 0; y < grid.extent(1) - 3; ++y) {
        for (int32_t x = 0; x < grid.extent(0) - 3; ++x) {
            count += CheckMatch<1, 1>(grid, y, x) ? 1 : 0;
            count += CheckMatch<-1, -1>(grid, y + 3, x + 3) ? 1 : 0;
            count += CheckMatch<1, -1>(grid, y, x + 3) ? 1 : 0;
            count += CheckMatch<-1, 1>(grid, y + 3, x) ? 1 : 0;
        }
    }
    return count;
}

size_t Part2(const InputGrid<const char> grid) {
    static constexpr auto countEq = [](char c, auto... corner) {
        return (((corner == c) ? 1 : 0) + ...);
    };
    size_t count{0};
    for (int32_t y = 1; y < grid.extent(1) - 1; ++y) {
        for (int32_t x = 1; x < grid.extent(0) - 1; ++x) {
            const char cc    = grid[y, x];
            const char tl    = grid[y - 1, x - 1];
            const char tr    = grid[y - 1, x + 1];
            const char bl    = grid[y + 1, x - 1];
            const char br    = grid[y + 1, x + 1];
            const int mCount = countEq('M', tl, tr, bl, br);
            const int sCount = countEq('S', tl, tr, bl, br);
            if (cc == 'A' && tl == br && mCount == 2 && sCount == 2) {
                ++count;
            }
        }
    }
    return count;
}

} // namespace

void AocMain(std::string_view input) {
    const mdspan grid = ToGrid(input);
    for (int i = 0; i < 5; ++i) {
        logger.solution("{}", StopWatch<std::micro>::Run("Part 1", Part1, grid));
        logger.solution("{}", StopWatch<std::micro>::Run("Part 2", Part2, grid));
    }
}