namespace {

struct Equation {
    uint64_t test{};
    boost::container::static_vector<uint64_t, 16> numbers;

    Equation(std::string_view line) {
        const std::size_t colon{line.find(':')};
        test    = ParseNumber<uint64_t>(line.substr(0, colon));
        numbers = line.substr(colon + 2) | Split(' ') | views::transform(ParseNumber<uint64_t>) |
                  ranges::to<decltype(numbers)>;
    }
};

constexpr uint64_t concat(uint64_t lhs, uint64_t rhs) {
    return lhs * Power(10, LogPlusOne<10>(rhs)) + rhs;
}

template <bool tryConcat>
bool SolveableImpl(const uint64_t test, std::span<const uint64_t> numbers, const uint64_t lhs) {
    if (numbers.empty()) {
        return test == lhs;
    }
    if (lhs >= test) {
        return false;
    }
    if (SolveableImpl<tryConcat>(test, numbers.subspan<1>(), lhs + numbers.front())) {
        return true;
    }
    if (SolveableImpl<tryConcat>(test, numbers.subspan<1>(), lhs * numbers.front())) {
        return true;
    }
    if constexpr (tryConcat) {
        if (SolveableImpl<tryConcat>(test, numbers.subspan<1>(), concat(lhs, numbers.front()))) {
            return true;
        }
    }
    return false;
}

template <bool tryConcat>
bool Solveable(const Equation& equation) {
    return SolveableImpl<tryConcat>(equation.test, std::span{equation.numbers}.subspan<1>(), equation.numbers.front());
}

std::vector<Equation> Parse(std::string_view input) {
    return input | Split('\n') | views::transform(Constructor<Equation>{}) | ranges::to_vector;
}

int64_t Part1(std::span<const Equation> equations) {
    return ranges::fold_left(equations | views::filter(Solveable<false>) | views::transform(&Equation::test),
                             std::int64_t{}, std::plus{});
}
int64_t Part2(std::span<const Equation> equations) {
    return ranges::fold_left(equations | views::filter(Solveable<true>) | views::transform(&Equation::test),
                             std::int64_t{}, std::plus{});
}

} // namespace

void AocMain(std::string_view input) {
    auto equations = StopWatch<std::micro>::Run("Parse", Parse, input);
    logger.solution("Part 1: {}", StopWatch<std::micro>::Run("Part 1", Part1, equations));
    logger.solution("Part 2: {}", StopWatch<std::milli>::Run("Part 2", Part2, equations));
}