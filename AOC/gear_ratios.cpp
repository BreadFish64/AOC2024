namespace {
constexpr std::bitset<256> SYMBOL_CHARACTERS = [] {
    std::bitset<256> val{};
    for (int c = 0; c < 256; ++c) {
        val[c] = IsPunct(c);
    }
    val.reset('.');
    return val;
}();

struct PartNumber {
    ssize number{};
    std::bitset<256> adjacent{};
    constexpr bool hasAdjacentSymbol() const { return (adjacent & SYMBOL_CHARACTERS).any(); }
};
} // namespace

void AocMain(std::string input) {
    const ssize width  = input.find('\n');
    const ssize stride = width + 1;
    const ssize height = input.size() / stride;

    std::map<std::pair<ssize, ssize>, std::vector<ssize>> gears;
    std::vector<PartNumber> partNumbers;

    const auto Access = [&](ssize y, ssize x) -> unsigned char {
        if (y < 0 || y >= height || x < 0 || x >= width) [[unlikely]] {
            return '.';
        }
        return input[y * stride + x];
    };

    const auto CheckAdjacent = [&](PartNumber& part, ssize y, ssize x) {
        const auto c = Access(y, x);
        if (c == '*') gears[{y, x}].emplace_back(part.number);
        part.adjacent.set(c);
    };

    const auto GetNext = [&](ssize pos) {
        return std::find_if(input.begin() + pos, input.end(), IsDigit) - input.begin();
    };

    for (ssize pos = GetNext(0); pos < std::ssize(input); pos = GetNext(pos)) {
        const ssize row = pos / stride;
        const ssize col = pos - row * stride;
        ssize end       = pos;
        for (ssize endcol = col; endcol < width && IsDigit(input[end]); ++endcol)
            ++end;
        const ssize length = end - pos;
        PartNumber partNumber{};
        std::from_chars(input.data() + pos, input.data() + end, partNumber.number);
        for (ssize x = col - 1; x < col + length + 1; ++x) {
            CheckAdjacent(partNumber, row - 1, x);
            CheckAdjacent(partNumber, row + 1, x);
        }
        CheckAdjacent(partNumber, row, col - 1);
        CheckAdjacent(partNumber, row, col + length);
        partNumbers.emplace_back(partNumber);
        pos = end;
    }
    fmt::print("Part 1: {}\n", ranges::accumulate(partNumbers | views::filter(&PartNumber::hasAdjacentSymbol), ssize{},
                                                  std::plus{}, &PartNumber::number));
    fmt::print("Part 2: {}\n", ranges::accumulate(gears | views::filter([](const auto& gear) {
                                                      const auto& [pos, ratios] = gear;
                                                      return ratios.size() == 2;
                                                  }) | views::values,
                                                  ssize{}, std::plus{},
                                                  [](std::span<const ssize> ratios) { return ratios[0] * ratios[1]; }));
}