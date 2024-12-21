void AocMain(std::string_view input) {
    auto split         = input.find("\n\n"sv);
    auto rulesString   = input.substr(0, split + 1);
    auto updatesString = input.substr(split + 2);

    auto parseRule = [](std::string_view line) {
        size_t bar = line.find('|');
        return std::make_pair(ParseNumber<uint32_t>(line.substr(0, bar)), ParseNumber<uint32_t>(line.substr(bar + 1)));
    };
    std::array<std::array<bool, 100>, 100> lessThan_{};
    for (auto [low, high] : rulesString | Split('\n') | views::transform(parseRule)) {
        lessThan_[high][low] = true;
    }
    auto orderedBefore = [&lessThan_](uint32_t lhs, uint32_t rhs) {
        return lessThan_[rhs][lhs];
    };
    auto getMiddlePage = [&orderedBefore](std::string_view line) -> uint64_t {
        auto pages = line | Split(',') | views::transform(ParseNumber<uint32_t>) |
                     ranges::to<boost::container::static_vector<uint32_t, 32>>;
        if (ranges::is_sorted(pages, orderedBefore)) {
            return pages[pages.size() / 2];
        } else {
            ranges::sort(pages, orderedBefore);
            return static_cast<uint64_t>(pages[pages.size() / 2]) << 32;
        }
    };
    auto parts =
        ranges::fold_left(updatesString | Split('\n') | views::transform(getMiddlePage), uint64_t{}, std::plus{});

    logger.solution("Part 1: {}", parts & ~uint32_t{});
    logger.solution("Part 2: {}", parts >> 32);
}