namespace {
ptrdiff_t Parse1(std::string_view calibrationValue) {
    return (*ranges::find_if(calibrationValue, IsDigit) - '0') * 10 +
           (*ranges::find_if(calibrationValue | views::reverse, IsDigit) - '0');
}

std::optional<ptrdiff_t> ParseNumberWord(std::string_view sv) {
    static constexpr std::array<std::string_view, 10> numbers{
        "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine",
    };
    auto it = ranges::find_if(numbers, [sv](std::string_view n) { return sv.starts_with(n); });
    return it == numbers.end() ? std::nullopt : std::make_optional(it - numbers.begin());
}

ptrdiff_t Parse2(std::string_view calibrationValue) {
    ptrdiff_t firstDigit = ranges::find_if(calibrationValue, IsDigit) - calibrationValue.begin();
    ptrdiff_t lastDigit =
        ranges::find_if(calibrationValue | views::reverse, IsDigit).base() - calibrationValue.begin() - 1;
    ptrdiff_t firstDigitValue = calibrationValue[firstDigit] - '0';
    ptrdiff_t lastDigitValue  = calibrationValue[lastDigit] - '0';
    for (ptrdiff_t i = 0; i < firstDigit; ++i) {
        if (auto number = ParseNumberWord(calibrationValue.substr(i))) {
            firstDigitValue = *number;
            break;
        }
    }
    for (ptrdiff_t i = calibrationValue.size() - 1; i > lastDigit; --i) {
        if (auto number = ParseNumberWord(calibrationValue.substr(i))) {
            lastDigitValue = *number;
            break;
        }
    }
    return firstDigitValue * 10 + lastDigitValue;
}
} // namespace

void AocMain(std::string input) {
    const auto calibrationValues = input | SplitWs | ranges::to<std::vector>;
    fmt::print("Part 1: {}\n", ranges::accumulate(calibrationValues, ptrdiff_t{}, std::plus{}, Parse1));
    fmt::print("Part 2: {}\n", ranges::accumulate(calibrationValues, ptrdiff_t{}, std::plus{}, Parse2));
}