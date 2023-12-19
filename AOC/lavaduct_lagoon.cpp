namespace {
constexpr u8 RIGHT = 0b00, DOWN = 0b01, LEFT = 0b10, UP = 0b11;

struct Edge {
    s64 steps;
    u8 direction;
};

s64 Solve(ranges::range auto&& edges) {
    s64 area{};
    s64 y{};
    for (const auto [steps, direction] : edges) {
        if (direction & 0b01) {
            y += (direction & 0b10) ? -steps : steps;
            area += steps;
        } else {
            area += ((direction & 0b10) ? 2 : -2) * y * steps + steps;
        }
    }
    return area / 2 + 1;
}

s64 Part1(std::string_view input) {
    return Solve(input | Split('\n') | views::transform([](std::string_view line) {
                     const size_t stepsLocation = line.find(' ') + 1;
                     const size_t secondSpace   = line.find(' ', stepsLocation);
                     return Edge{
                         .steps = ParseNumber<s64>(line.substr(stepsLocation, secondSpace - stepsLocation)),
                         .direction =
                             [&] {
                                 switch (line.front()) {
                                     case 'R': return RIGHT;
                                     case 'D': return DOWN;
                                     case 'L': return LEFT;
                                     case 'U': return UP;
                                     default: Assume(false);
                                 }
                                 return u8{};
                             }(),
                     };
                 }));
}

s64 Part2(std::string_view input) {
    return Solve(input | Split('\n') | views::transform([](std::string_view line) {
                     const size_t hexLocation = line.find(' ', line.find(' ') + 1) + 3;
                     return Edge{
                         .steps     = ParseNumber<s64>(line.substr(hexLocation, 5), 16),
                         .direction = static_cast<u8>(line[hexLocation + 5] - '0'),
                     };
                 }));
}
} // namespace

void AocMain(std::string_view input) {
    logger.solution("Part 1 {}", StopWatch<std::micro>::Run("Part 1", Part1, input));
    logger.solution("Part 2 {}", StopWatch<std::micro>::Run("Part 2", Part2, input));
}