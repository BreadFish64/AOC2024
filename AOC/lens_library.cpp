namespace {
struct Lens {
    u32 label{};
    u32 focalLength{};
};
u8 Hash(std::string_view step) {
    u8 hash{};
    for (const u8 c : step) {
        hash += c;
        hash *= 17;
    }
    return hash;
}
u32 UniqueHash(std::string_view step) {
    u32 num{};
    for (const char c : step) {
        num <<= 5;
        num += (c - 'A') + 1;
    }
    return num;
}
} // namespace

void AocMain(std::string_view input) {
    const auto sequence = input | views::drop_last(1) | Split(',');
    logger.solution("Part 1: {}", ranges::accumulate(sequence, u64{}, std::plus{}, Hash));
    std::array<boost::container::small_vector<Lens, 4>, 256> boxes{};
    for (std::string_view step : sequence) {
        if (step.back() == '-') {
            const auto labelStr = step.substr(0, step.size() - 1);
            const u32 label     = UniqueHash(labelStr);
            auto& box           = boxes[Hash(labelStr)];
            auto it             = ranges::find_if(box, [label](const auto& lens) { return lens.label == label; });
            if (it != box.end()) {
                box.erase(it);
            }
        } else {
            const size_t eqPos    = step.find('=');
            const auto labelStr   = step.substr(0, eqPos);
            const u32 focalLength = ParseNumber<u32>(step.substr(eqPos + 1));
            const u32 label       = UniqueHash(labelStr);
            auto& box             = boxes[Hash(labelStr)];
            auto it               = ranges::find_if(box, [label](const auto& lens) { return lens.label == label; });
            if (it != box.end()) {
                it->focalLength = focalLength;
            } else {
                box.emplace_back(label, focalLength);
            }
        }
    }
    size_t focusingPower{};
    for (size_t boxIdx = 0; boxIdx < boxes.size(); ++boxIdx) {
        const auto& box = boxes[boxIdx];
        for (size_t lensIdx = 0; lensIdx < box.size(); ++lensIdx) {
            u64 lensPower = (boxIdx + 1) * (lensIdx + 1) * box[lensIdx].focalLength;
            logger.info("box: {} * lens: {} * focal length: {} == {}", boxIdx, lensIdx, box[lensIdx].focalLength,
                        lensPower);
            focusingPower += lensPower;
        }
    }
    logger.solution("Part 2: {}", focusingPower);
}