namespace {

struct Terrain {
    using Slice                          = u32;
    static constexpr unsigned MAX_SLICES = 32;

    std::array<Slice, MAX_SLICES> horizontal{};
    std::array<Slice, MAX_SLICES> vertical{};
    unsigned height{};
    unsigned width{};

    template <bool RIGHT_SIZE>
    static bool HasSmudgeCount(const std::span<const Slice> slices, unsigned width, unsigned smudgeTarget,
                               unsigned mirrorSize) {
        smudgeTarget *= 2; // mirroring and xoring will flip two bits per smudge
        Slice mask = (Slice{1} << mirrorSize) - 1;
        mask |= bitreverse(mask);
        Slice smudges{};
        for (const Slice slice : slices) {
            const Slice rotated =
                RIGHT_SIZE ? std::rotl(std::rotr(slice, width), mirrorSize) : std::rotr(slice, mirrorSize);
            const Slice masked    = rotated & mask;
            const Slice reflected = bitreverse(masked);
            smudges += std::popcount(masked ^ reflected);
            if (smudges > smudgeTarget) return false;
        }
        return smudges == smudgeTarget;
    }

    static std::optional<unsigned> FindMirrorOnAxis(const std::span<const Slice> slices, const unsigned width,
                                                    const unsigned smudgeTarget) {
        for (unsigned mirrorSize = 1; mirrorSize <= width / 2; ++mirrorSize) {
            if (HasSmudgeCount<false>(slices, width, smudgeTarget, mirrorSize)) return mirrorSize;
            if (HasSmudgeCount<true>(slices, width, smudgeTarget, mirrorSize)) return width - mirrorSize;
        }
        return std::nullopt;
    }

    template <bool SMUDGES>
    unsigned findMirror() const {
        if (auto mirrorPos = FindMirrorOnAxis(std::span{horizontal}.first(height), width, SMUDGES ? 1 : 0)) {
            return *mirrorPos;
        }
        if (auto mirrorPos = FindMirrorOnAxis(std::span{vertical}.first(width), height, SMUDGES ? 1 : 0)) {
            return *mirrorPos * 100;
        }
        throw std::logic_error{"no mirror"};
    }

    static Terrain parse(std::string_view expanded) {
        Terrain compact{};
        const size_t size   = expanded.size() + (expanded.ends_with('\n') ? 0 : 1);
        compact.width       = expanded.find('\n');
        const size_t stride = compact.width + 1;
        compact.height      = size / stride;
        for (size_t y = 0; y < compact.height; ++y) {
            for (size_t x = 0; x < compact.width; ++x) {
                const bool rock = expanded[y * stride + x] == '#';
                compact.horizontal[y] |= Slice{rock} << x;
                compact.vertical[x] |= Slice{rock} << y;
            }
        }
        return compact;
    }
};

template <bool SMUDGES>
u64 Part(const std::span<const Terrain> terrains) {
    return ranges::accumulate(terrains, u64{}, std::plus{}, &Terrain::findMirror<SMUDGES>);
}

std::vector<Terrain> Parse(std::string_view input) {
    return input | Split("\n\n"sv) | views::transform(Terrain::parse) | ranges::to<std::vector>;
}

} // namespace

void AocMain(std::string_view input) {
    for (int i = 0; i < 100; ++i) {
        const std::vector<Terrain> terrains = StopWatch<std::micro>::Run("Parse", Parse, input);
        logger.info("Max height: {}", ranges::max(terrains, std::less{}, &Terrain::height).height);
        logger.info("Max width: {}", ranges::max(terrains, std::less{}, &Terrain::width).width);
        logger.solution("Part 1: {}\n", StopWatch<std::micro>::Run("Part 1", Part<false>, std::span{terrains}));
        logger.solution("Part 2: {}\n", StopWatch<std::micro>::Run("Part 2", Part<true>, std::span{terrains}));
    }
}