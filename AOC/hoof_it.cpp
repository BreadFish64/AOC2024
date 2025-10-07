#include "Mdspan.hpp"
namespace {

int16_t ScoreTraverse(InputGrid<const char> map, mdspan<uint8_t, dextents<int32_t, 2>> traversedMap, Pos2D pos,
                      char target) {
    if (!InBounds(map, pos)) {
        return 0;
    }
    const char cell{map[pos]};
    if (cell != target) {
        return 0;
    }
    if (std::exchange(traversedMap[pos], true)) {
        return 0;
    }
    if (cell == '9') {
        return 1;
    }
    int16_t score{};
    score += ScoreTraverse(map, traversedMap, pos + Vec2D{-1, 0}, target + 1);
    score += ScoreTraverse(map, traversedMap, pos + Vec2D{0, -1}, target + 1);
    score += ScoreTraverse(map, traversedMap, pos + Vec2D{0, 1}, target + 1);
    score += ScoreTraverse(map, traversedMap, pos + Vec2D{1, 0}, target + 1);
    return score;
}

int16_t RateTraverse(InputGrid<const char> map, mdspan<int16_t, dextents<int32_t, 2>> traversedMap, Pos2D pos,
                     char target) {
    if (!InBounds(map, pos)) {
        return 0;
    }
    if (map[pos] != target) {
        return 0;
    }
    if (traversedMap[pos]) {
        return (traversedMap[pos] < 0) ? 0 : traversedMap[pos];
    }
    int16_t rating{0};
    rating += RateTraverse(map, traversedMap, pos + Vec2D{-1, 0}, target + 1);
    rating += RateTraverse(map, traversedMap, pos + Vec2D{0, -1}, target + 1);
    rating += RateTraverse(map, traversedMap, pos + Vec2D{0, 1}, target + 1);
    rating += RateTraverse(map, traversedMap, pos + Vec2D{1, 0}, target + 1);
    traversedMap[pos] = (rating == 0) ? -1 : rating;
    return rating;
}

int64_t TotalScore(std::string_view input) {
    const mdspan map = ToGrid(input);
    int64_t totalScore{0};
    for (size_t trailheadIndex = input.find('0'); trailheadIndex < input.size();
         trailheadIndex        = input.find('0', trailheadIndex + 1)) {
        mdarray<uint8_t, dextents<int32_t, 2>> traversedMap(map.extents());
        Pos2D pos{
            static_cast<int32_t>(trailheadIndex / map.stride(0)),
            static_cast<int32_t>(trailheadIndex % map.stride(0)),
        };
        int16_t score{ScoreTraverse(map, traversedMap, pos, '0')};
        totalScore += score;
    }
    return totalScore;
}

int64_t TotalRating(std::string_view input) {
    const mdspan map = ToGrid(input);
    mdarray<int16_t, dextents<int32_t, 2>> masterTraverseMap(map.extents());
    for (size_t peakIndex = input.find('9'); peakIndex < input.size(); peakIndex = input.find('9', peakIndex + 1)) {
        Pos2D pos{
            static_cast<int32_t>(peakIndex / map.stride(0)),
            static_cast<int32_t>(peakIndex % map.stride(0)),
        };
        masterTraverseMap.to_mdspan()[pos] = 1;
    }

    int64_t totalRating{0};
    for (size_t trailheadIndex = input.find('0'); trailheadIndex < input.size();
         trailheadIndex        = input.find('0', trailheadIndex + 1)) {
        mdarray traversedMap = masterTraverseMap;
        Pos2D pos{
            static_cast<int32_t>(trailheadIndex / map.stride(0)),
            static_cast<int32_t>(trailheadIndex % map.stride(0)),
        };
        int16_t rating{RateTraverse(map, traversedMap, pos, '0')};
        totalRating += rating;
    }
    return totalRating;
}

} // namespace

void AocMain(std::string_view input) {
    logger.solution("TotalScore:  {}", StopWatch<std::micro>::Run("TotalScore", TotalScore, input));
    logger.solution("TotalRating: {}", StopWatch<std::micro>::Run("TotalRating", TotalRating, input));
}