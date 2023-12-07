namespace {

struct Hand {
    static constexpr size_t SIZE = 5;

    enum Strength {
        HIGH_CARD,
        ONE_PAIR,
        TWO_PAIR,
        THREE_OF_A_KIND,
        FULL_HOUSE,
        FOUR_OF_A_KIND,
        FIVE_OF_A_KIND,
    };

    std::array<s8, SIZE> cards{};
    s64 bid{};
    Strength strength{};

    friend std::strong_ordering operator<=>(const Hand& lhs, const Hand& rhs) {
        if (std::strong_ordering relativeHandStrength = lhs.strength <=> rhs.strength;
            relativeHandStrength != std::strong_ordering::equal) {
            return relativeHandStrength;
        }
        return std::lexicographical_compare_three_way(lhs.cards.begin(), lhs.cards.end(), rhs.cards.begin(),
                                                      rhs.cards.end());
    }

    template <bool PART2>
    static s8 CardToNumber(char card) {
        if (IsDigit(card)) {
            return card - '0';
        } else {
            switch (card) {
                case 'T': return 10;
                case 'J': return PART2 ? 1 : 11;
                case 'Q': return 12;
                case 'K': return 13;
                case 'A': return 14;
                default: [[unlikely]] throw std::invalid_argument{std::string{card}};
            }
        }
    }

    template <bool PART2>
    static std::array<s8, SIZE> CardsToNumbers(std::string_view cards) {
        std::array<s8, SIZE> numbers{};
        ranges::transform(cards | views::take(SIZE), numbers.begin(), CardToNumber<PART2>);
        return numbers;
    }

    template <bool PART2>
    static Strength CalculateHandStrength(std::span<const s8, SIZE> cards) {
        std::array<s8, 15> cardCounts{};
        for (const s8 card : cards)
            ++cardCounts[card];

        std::array<s8, 6> ofAKindCounts{};
        for (const s8 kind : cardCounts | views::drop(2))
            ++ofAKindCounts[kind];

        if constexpr (PART2) {
            if (ofAKindCounts[5] >= 1) return FIVE_OF_A_KIND;
            if (ofAKindCounts[4] >= 1) return FOUR_OF_A_KIND;
            if (ofAKindCounts[3] >= 1 && ofAKindCounts[2] >= 1) return FULL_HOUSE;
            if (ofAKindCounts[3] >= 1) return THREE_OF_A_KIND;
            if (ofAKindCounts[2] >= 2) return TWO_PAIR;
            if (ofAKindCounts[2] >= 1) return ONE_PAIR;
        } else {
            const int jokerCount     = cardCounts[1];
            const auto CanBeXOfAKind = [&](int x) {
                int requiredJokers = 0;
                while (x >= 0 && requiredJokers <= jokerCount) {
                    if (ofAKindCounts[x] >= 1) return true;
                    --x;
                    ++requiredJokers;
                }
                return false;
            };

            if (CanBeXOfAKind(5)) return FIVE_OF_A_KIND;
            if (CanBeXOfAKind(4)) return FOUR_OF_A_KIND;
            if ((ofAKindCounts[3] >= 1 && ofAKindCounts[2] >= 1) || (ofAKindCounts[2] >= 2 && jokerCount > 0))
                return FULL_HOUSE;
            if (CanBeXOfAKind(3)) return THREE_OF_A_KIND;
            if (ofAKindCounts[2] >= 2) return TWO_PAIR;
            if (CanBeXOfAKind(2)) return ONE_PAIR;
        }
        return HIGH_CARD;
    }

    template <bool PART2>
    static Hand Parse(std::string_view handStr) {
        auto split       = handStr.find(' ');
        const auto cards = CardsToNumbers<PART2>(handStr.substr(0, split));
        return {.cards    = cards,
                .bid      = ParseNumber<s64>(handStr.substr(split + 1)),
                .strength = CalculateHandStrength<PART2>(cards)};
    }
};

template <bool PART2>
std::vector<Hand> Parse(std::string_view input) {
    return input | Split('\n') | views::transform(Hand::Parse<PART2>) | ranges::to<std::vector>;
}

s64 Solve(std::span<Hand> hands) {
    std::sort(hands.begin(), hands.end());
    s64 rank  = 1;
    s64 score = 0;
    for (const Hand& hand : hands) {
        score += hand.bid * rank;
        ++rank;
    }
    return score;
}

} // namespace

void AocMain(std::string_view input) {
    constexpr auto test = R"(32T3K 765
T55J5 684
KK677 28
KTJJT 220
QQQJA 483
)"sv;
    for (std::string_view in : {test, input}) {
        auto hands1 = StopWatch<std::milli>::Run("Parse1", Parse<false>, in);
        auto hands2 = StopWatch<std::milli>::Run("Parse2", Parse<true>, in);
        logger.solution("Part 1: {}", StopWatch<std::milli>::Run("Part1", Solve, hands1));
        logger.solution("Part 2: {}", StopWatch<std::milli>::Run("Part2", Solve, hands2));
    }
}