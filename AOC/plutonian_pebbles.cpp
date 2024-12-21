#include <boost/unordered/unordered_map.hpp>

namespace {

template <size_t depth>
boost::unordered::unordered_flat_map<uint64_t, uint64_t> cache;

template <size_t depth>
size_t CountExpanded(uint64_t pebble) {
    if constexpr (depth == 0) {
        return 1;
    } else {
        auto it = cache<depth>.find(pebble);
        if (it != cache<depth>.end()) {
            return it->second;
        }
        size_t ret = [&] {
            if (pebble == 0) {
                return CountExpanded<depth - 1>(1);
            }
            unsigned digits = LogPlusOne<10>(pebble);
            if (digits % 2 == 0) {
                size_t power = Power(10_size_t, digits / 2);
                return CountExpanded<depth - 1>(pebble / power) + CountExpanded<depth - 1>(pebble % power);
            }
            return CountExpanded<depth - 1>(pebble * 2024);
        }();
        cache<depth>.emplace(pebble, ret);
        return ret;
    }
}

template <size_t depth>
size_t DoPart(std::string_view input) {
    return ranges::fold_left(input | ParseNumbers<size_t> | views::transform(CountExpanded<depth>), size_t{},
                             std::plus{});
}

size_t CountExpanded2(const uint64_t initialPebble) {
    boost::unordered_flat_map<uint64_t, size_t> pebbles{{initialPebble, 1}};
    boost::unordered_flat_map<uint64_t, size_t> newPebbles;
    for (int i = 0; i < 75; ++i) {
        for (auto [pebble, count] : pebbles) {
            if (pebble == 0) {
                newPebbles[1] += count;
            } else {
                unsigned digits = LogPlusOne<10>(pebble);
                if (digits % 2 == 0) {
                    size_t power = Power(10_size_t, digits / 2);
                    newPebbles[pebble / power] += count;
                    newPebbles[pebble % power] += count;
                } else {
                    newPebbles[pebble * 2024] += count;
                }
            }
        }
        pebbles.swap(newPebbles);
        newPebbles.clear();
    }
    return ranges::fold_left(pebbles | views::transform(&std::pair<const uint64_t, size_t>::second), size_t{}, std::plus{});
}

size_t DoPart2(std::string_view input) {
    return ranges::fold_left(input | ParseNumbers<size_t> | views::transform(CountExpanded2), size_t{},
                             std::plus{});
}

} // namespace

void AocMain(std::string_view input) {
    logger.solution("25 Blinks: {}", StopWatch<std::micro>::Run("25", DoPart<25>, input));
    logger.solution("75 Blinks: {}", StopWatch<std::milli>::Run("75", DoPart<75>, input));
    logger.solution("75 Blinks no recursion: {}", StopWatch<std::milli>::Run("75 no recursion", DoPart2, input));
}