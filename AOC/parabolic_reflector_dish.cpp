namespace {

template <ssize Y_OFFSET, ssize X_OFFSET>
void Drop(const auto& state, auto&& yrange, auto&& xrange) {
    bool modified = true;
    while (modified) {
        modified = false;
        for (ssize y : yrange) {
            for (ssize x : xrange) {
                char& dst = state(y + Y_OFFSET, x + X_OFFSET);
                char& src = state(y, x);
                if (src == 'O' && dst == '.') {
                    modified = true;
                    dst      = 'O';
                    src      = '.';
                }
            }
        }
    }
}
void DropNorth(const auto& state) {
    Drop<-1, 0>(state, views::iota(ssize{1}, state.extent(0)), views::iota(ssize{0}, state.extent(1)));
}
void DropWest(const auto& state) {
    Drop<0, -1>(state, views::iota(ssize{0}, state.extent(0)), views::iota(ssize{1}, state.extent(1)));
}
void DropSouth(const auto& state) {
    Drop<1, 0>(state, views::iota(ssize{0}, state.extent(0) - 1) | views::reverse,
               views::iota(ssize{0}, state.extent(1)));
}
void DropEast(const auto& state) {
    Drop<0, 1>(state, views::iota(ssize{0}, state.extent(0)),
               views::iota(ssize{0}, state.extent(1) - 1) | views::reverse);
}

u64 CalculateLoad(const auto& state) {
    u64 load{};
    for (ssize y = 0; y < state.extent(0); ++y) {
        for (ssize x = 0; x < state.extent(1); ++x) {
            if (state(y, x) == 'O') {
                load += state.extent(0) - y;
            }
        }
    }
    return load;
}

} // namespace

void AocMain(std::string_view input) {
    std::string stateStorage{input};
    const ssize width        = stateStorage.find('\n');
    const auto MakeStateView = [width]<typename S>(S& storage) {
        using MaybeConstChar = std::remove_reference_t<ranges::range_reference_t<S>>;
        const ssize stride   = width + 1;
        const std::mdspan<MaybeConstChar, std::dextents<ssize, 2>> stateWithNewline{
            storage.data(), std::ssize(storage) / stride, stride};
        return std::submdspan(stateWithNewline, std::full_extent, std::pair{std::integral_constant<ssize, 0>{}, width});
    };
    boost::unordered_flat_map<std::string, s64> cache;
    s64 cycles = 0;
    s64 loopStart{};
    {
        const auto state    = MakeStateView(stateStorage);
        cache[stateStorage] = cycles;
        while (true) {
            DropNorth(state);
            if (cycles == 0) {
                logger.solution("{}\n", CalculateLoad(state));
            }
            DropWest(state);
            DropSouth(state);
            DropEast(state);
            ++cycles;
            auto [it, isNew] = cache.try_emplace(stateStorage, cycles);
            if (!isNew) {
                loopStart = it->second;
                break;
            }
        }
    }
    s64 loopLength   = cycles - loopStart;
    s64 indexInCycle = (1000000000 - loopStart) % loopLength;
    s64 loadCycle    = indexInCycle + loopStart;
    std::string_view loadStateStorage =
        ranges::find_if(cache, [loadCycle](const auto& it) { return it.second == loadCycle; })->first;
    logger.solution("{}", CalculateLoad(MakeStateView(loadStateStorage)));
}