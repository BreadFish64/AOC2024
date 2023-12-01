namespace {
void Part1(std::span<const int> frequencyChanges) {
    fmt::print("Part 1: {}\n", ranges::accumulate(frequencyChanges, int{}));
}

void Part2(std::span<const int> frequencyChanges) {
    boost::unordered_flat_set<int> absoluteFrequencies{0};
    int currentFrequency = 0;
    while (true) {
        for (const int change : frequencyChanges) {
            currentFrequency += change;
            const auto [it, isNew] = absoluteFrequencies.insert(currentFrequency);
            if (!isNew) {
                fmt::print("Part 2: {}\n", currentFrequency);
                return;
            }
        }
    }
}
} // namespace

void AocMain(std::string input) {
    const auto frequencyChanges = input | ParseNumbers<int> | ranges::to<std::vector>;
    Part1(frequencyChanges);
    Part2(frequencyChanges);
}