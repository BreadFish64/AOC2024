namespace {

int64_t Run1(std::string_view input) {
    int64_t sum{0};
    while (!input.empty()) {
        std::size_t mulPos = input.find("mul("sv);
        if (mulPos == input.npos) {
            break;
        }
        input.remove_prefix(mulPos + "mul("sv.size());

        std::size_t numberLen = ranges::find_if_not(input, IsDigit) - input.begin();
        if (numberLen == 0 || numberLen > 3 || numberLen == input.size() || input[numberLen] != ',') {
            input.remove_prefix(numberLen);
            continue;
        }
        auto lhs = ParseNumber<int64_t>(input.substr(0, numberLen));
        input.remove_prefix(numberLen + 1);

        numberLen = ranges::find_if_not(input, IsDigit) - input.begin();
        if (numberLen == 0 || numberLen > 3 || numberLen == input.size() || input[numberLen] != ')') {
            input.remove_prefix(numberLen);
            continue;
        }
        auto rhs = ParseNumber<int64_t>(input.substr(0, numberLen));
        input.remove_prefix(numberLen + 1);

        sum += lhs * rhs;
    }
    return sum;
}

int64_t Run2(std::string_view input) {
    int64_t sum{0};
    while (true) {
        {
            std::size_t dontPos = input.find("don't()"sv);
            sum += Run1(input.substr(0, dontPos));
            if (dontPos == input.npos) {
                break;
            }
            input.remove_prefix(dontPos + "don't()"sv.size());
        }
        {
            std::size_t doPos = input.find("do()"sv);
            if (doPos == input.npos) {
                break;
            }
            input.remove_prefix(doPos + "do()"sv.size());
        }
    }
    return sum;
}

} // namespace

void AocMain(std::string_view input) {
    logger.solution("{}", StopWatch<std::micro>::Run("Part 1", Run1, input));
    logger.solution("{}", StopWatch<std::micro>::Run("Part 2", Run2, input));
}
