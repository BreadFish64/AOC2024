namespace {

constexpr uint32_t IterateSecret(uint32_t x) {
    x = (x ^ (x << 6)) & 0x00FFFFFF;
    x = (x ^ (x >> 5)) & 0x00FFFFFF;
    x = (x ^ (x << 11)) & 0x00FFFFFF;
    return x;
}
static_assert(IterateSecret(123) == 15887950);

uint64_t Part1(std::span<const uint32_t> buyers) {
    return ranges::accumulate(buyers, uint64_t{0}, std::plus{}, [](uint32_t x) {
        for (int i = 0; i < 2000; ++i) {
            x = IterateSecret(x);
        }
        return x;
    });
}

boost::unordered_flat_map<std::array<int8_t, 4>, int8_t> EncodeBuyer(uint32_t secret) {
    boost::unordered_flat_map<std::array<int8_t, 4>, int8_t> priceMap;
    std::array<int8_t, 4> changeSequence{};
    int8_t previousPrice{0};
    const auto Push = [&](auto updateMap) {
        int8_t price      = secret % 10;
        int8_t priceDelta = price - previousPrice;
        
        ranges::rotate(changeSequence, changeSequence.begin() + 1);
        changeSequence.back() = priceDelta;

        if (updateMap) {
            priceMap.try_emplace(changeSequence, price);
        }

        previousPrice = price;
        secret        = IterateSecret(secret);
    };
    Push(std::false_type{});
    Push(std::false_type{});
    Push(std::false_type{});
    Push(std::false_type{});
    for (int i = 4; i <= 2000; ++i) {
        Push(std::true_type{});
    }
    return priceMap;
}

int64_t Part2(std::span<const uint32_t> buyers) {
    boost::unordered_flat_map<std::array<int8_t, 4>, int64_t> totalPriceMap;
    for (const uint32_t buyer : buyers) {
        auto priceMap = EncodeBuyer(buyer);
        for (const auto [changeSequence, price] : priceMap) {
            totalPriceMap[changeSequence] += price;
        }
    }
    return ranges::max_element(totalPriceMap, std::less{}, [](const auto& e) { return e.second; })->second;
}

} // namespace

void AocMain(std::string_view input) {
    std::vector<uint32_t> buyers = input | ParseNumbers<uint32_t> | ranges::to_vector;
    logger.solution("Part 1: {}", StopWatch<std::milli>::Run("Part 1", Part1, buyers));
    logger.solution("Part 2: {}", StopWatch<std::milli>::Run("Part 2", Part2, buyers));
}