namespace {
struct Race {
    s64 time;
    s64 distance;
};

s64 CountWinCondition(const Race& race) {
    const auto [lower, upper] = SolveQuadratic(-1.0, race.time, -race.distance);
    // Refine the limits with a loop because
    // I don't feel like figuring out a mathematical way to get this right
    s64 iupper = static_cast<s64>(upper) + 1;
    s64 ilower = static_cast<s64>(lower);
    while ((iupper * (race.time - iupper)) <= race.distance)
        --iupper;
    while ((ilower * (race.time - ilower)) <= race.distance)
        ++ilower;
    return iupper - ilower + 1; // inclusive
}

s64 Part1(std::span<const Race> races) {
    return ranges::accumulate(races, s64{1}, std::multiplies{}, CountWinCondition);
}

std::tuple<std::vector<Race>, Race> Parse(std::string_view input) {
    const auto& inputLines = input | Split('\n');
    auto lineIt            = inputLines.begin();

    std::string_view timeLine = *lineIt++;
    timeLine.remove_prefix(timeLine.find(':') + 1);
    std::string_view distanceLine = *lineIt++;
    distanceLine.remove_prefix(distanceLine.find(':') + 1);

    std::vector<Race> part1Races = ranges::zip_view(timeLine | ParseNumbers<s64>, distanceLine | ParseNumbers<s64>) |
                                   views::transform([](const auto& pair) {
                                       const auto [time, distance] = pair;
                                       return Race{time, distance};
                                   }) |
                                   ranges::to<std::vector>;
    Race part2Race{
        ParseNumber<s64>(timeLine | views::filter(IsGraph) | ranges::to<std::string>),
        ParseNumber<s64>(distanceLine | views::filter(IsGraph) | ranges::to<std::string>),
    };
    return std::make_tuple(std::move(part1Races), part2Race);
}
} // namespace

void AocMain(std::string_view input) {
    const auto [part1Races, part2Race] = StopWatch<std::micro>::Run("Parse", Parse, input);
    logger.solution("Part 1: {}", StopWatch<std::nano>::Run("Part1", Part1, part1Races));
    logger.solution("Part 2: {}", StopWatch<std::nano>::Run("Part2", CountWinCondition, part2Race));
}