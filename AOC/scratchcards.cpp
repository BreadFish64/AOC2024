namespace {
struct Card {
    std::bitset<100> winningNumbers;
    std::bitset<100> numbersYouHave;
    usize copies{1};

    size_t matches() const { return (winningNumbers & numbersYouHave).count(); }
    size_t score() const { return (1_sz << matches()) >> 1; }
};

void Part1(std::span<const Card> cards) {
    fmt::print("Part 1: {}\n", ranges::accumulate(cards, usize{}, std::plus{}, &Card::score));
}

void Part2(std::span<Card> cards) {
    size_t totalCards = 0;
    for (size_t i = 0; i < cards.size(); ++i) {
        const auto matches   = cards[i].matches();
        const size_t copyEnd = std::min(i + matches + 1, cards.size());
        for (size_t j = i + 1; j < copyEnd; ++j) {
            cards[j].copies += cards[i].copies;
        }
        totalCards += cards[i].copies;
    }
    fmt::print("Part 2: {}\n", totalCards);
}

std::bitset<100> MakeNumberSet(std::string_view str) {
    return ranges::accumulate(str | ParseNumbers<int>, std::bitset<100>{}, std::bit_or{}, [](int n) {
        std::bitset<100> set_pos{};
        set_pos.set(n);
        return set_pos;
    });
}
} // namespace

void AocMain(std::string input) {
    auto cards = input | Split('\n') | views::transform([](std::string_view cardLine) {
                     cardLine.remove_prefix(cardLine.find(':') + 2); // Card XXX:
                     auto bar = cardLine.find('|');
                     return Card{
                         .winningNumbers = MakeNumberSet(cardLine.substr(0, bar - 1)),
                         .numbersYouHave = MakeNumberSet(cardLine.substr(bar + 2)),
                     };
                 }) |
                 ranges::to<std::vector>;
    Part1(cards);
    Part2(cards);
}