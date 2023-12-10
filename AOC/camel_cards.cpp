namespace {

struct Hand {
    static constexpr size_t SIZE = 5;

    enum Strength : u8 {
        HIGH_CARD,
        ONE_PAIR,
        TWO_PAIR,
        THREE_OF_A_KIND,
        FULL_HOUSE,
        FOUR_OF_A_KIND,
        FIVE_OF_A_KIND,
    };

    s16 bid{};
    std::array<u8, SIZE> cards{};
    Strength strength{};

    [[gnu::always_inline]] friend std::strong_ordering operator<=>(const Hand lhs, const Hand rhs) {
        static_assert(std::endian::native == std::endian::little);
        return std::bit_cast<u64>(lhs) <=> std::bit_cast<u64>(rhs);
    }

    static u8 CardToNumber(char card, bool jokers) {
        if (IsDigit(card)) {
            return card - '0';
        } else {
            switch (card) {
                case 'T': return 10;
                case 'J': return jokers ? 1 : 11;
                case 'Q': return 12;
                case 'K': return 13;
                case 'A': return 14;
                default: [[unlikely]] throw std::invalid_argument{std::string{card}};
            }
        }
    }

    static std::array<u8, SIZE> CardsToNumbers(std::string_view cards, bool jokers) {
        std::array<u8, SIZE> numbers{}; // Reverse to form single little endian number
        ranges::transform(cards | views::take(SIZE), numbers.rbegin(),
                          [jokers](char card) { return CardToNumber(card, jokers); });
        return numbers;
    }

    static Strength CalculateHandStrength(std::span<const u8, SIZE> cards, bool jokers) {
        std::array<u8, 15> cardCounts{};
        for (const u8 card : cards)
            ++cardCounts[card];

        if (jokers) {
            const auto max = ranges::max_element(cardCounts | views::drop(2));
            *max += cardCounts[1];
        }

        std::array<u8, SIZE + 1> ofAKindCounts{};
        for (const u8 kind : cardCounts | views::drop(2))
            ++ofAKindCounts[kind];

        if (ofAKindCounts[5] >= 1) return FIVE_OF_A_KIND;
        if (ofAKindCounts[4] >= 1) return FOUR_OF_A_KIND;
        if (ofAKindCounts[3] >= 1 && ofAKindCounts[2] >= 1) return FULL_HOUSE;
        if (ofAKindCounts[3] >= 1) return THREE_OF_A_KIND;
        if (ofAKindCounts[2] >= 2) return TWO_PAIR;
        if (ofAKindCounts[2] >= 1) return ONE_PAIR;
        return HIGH_CARD;
    }

    static std::tuple<Hand, Hand> Parse(std::string_view handStr) {
        const auto split       = handStr.find(' ');
        const auto hand        = handStr.substr(0, split);
        const auto bid         = ParseNumber<s16>(handStr.substr(split + 1));
        const auto cards       = CardsToNumbers(hand, false);
        const auto cardsWJoker = CardsToNumbers(hand, true);
        return {
            {.bid = bid, .cards = cards, .strength = CalculateHandStrength(cards, false)},
            {.bid = bid, .cards = cardsWJoker, .strength = CalculateHandStrength(cardsWJoker, false)},
        };
    }
};

std::tuple<std::vector<Hand>, std::vector<Hand>> Parse(std::string_view input) {
    return input | Split('\n') | views::transform(Hand::Parse) |
           ranges::to<std::vector>;
}

s64 Solve(std::span<Hand> hands) {
    std::sort(hands.begin(), hands.end());
    s64 rank  = 1;
    s64 score = 0;
    for (const Hand hand : hands) {
        std::string str{"test string"};
        logger.info("{} {} {} {}", hand.cards, hand.bid, str, str.c_str());
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
        auto hands1 = StopWatch<std::micro>::Run("Parse1", Parse, in, false);
        auto hands2 = StopWatch<std::micro>::Run("Parse2", Parse, in, true);
        logger.solution("Part 1: {}", StopWatch<std::micro>::Run("Part1", Solve, hands1));
        logger.solution("Part 2: {}", StopWatch<std::micro>::Run("Part2", Solve, hands2));
    }
}