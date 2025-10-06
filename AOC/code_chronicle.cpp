namespace {

template <bool isKey>
std::array<std::int8_t, 5> ParseLockOrKey(std::string_view input) {
    std::array<std::int8_t, 5> init{};
    for (auto [y, line] : views::enumerate(input | Split('\n'))) {
        for (size_t x{0}; x < 5; ++x) {
            init[x] = (line[x] == (isKey ? '.' : '#')) ? static_cast<int8_t>(isKey ? (5 - y) : y) : init[x];
        }
    }
    return init;
}

bool Fits(std::array<std::int8_t, 5> key, std::array<std::int8_t, 5> lock) {
    bool fits{true};
    for (size_t x{0}; x < 5; ++x) {
        fits &= (key[x] + lock[x]) <= 5;
    }
    return fits;
}

} // namespace

void AocMain(std::string_view input) {
    std::vector<std::array<std::int8_t, 5>> locks;
    locks.reserve(input.size() / (8 * 6));
    std::vector<std::array<std::int8_t, 5>> keys;
    locks.reserve(keys.size() / (8 * 6));

    for (std::string_view lockOrKey : input | Split("\n\n"sv)) {
        if (lockOrKey.front() == '#') {
            locks.emplace_back(ParseLockOrKey<false>(lockOrKey));
        } else if (lockOrKey.front() == '.') {
            keys.emplace_back(ParseLockOrKey<true>(lockOrKey));
        } else {
            std::unreachable();
        }
    }

    logger.solution("{}", ranges::count_if(views::cartesian_product(keys, locks), [](auto&& combo) {
                        // I should really write an apply_view...
                        auto&& [key, lock] = combo;
                        return Fits(key, lock);
                    }));
}