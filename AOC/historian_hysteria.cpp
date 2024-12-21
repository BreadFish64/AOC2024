void AocMain(std::string_view input) {
    std::vector<int32_t> leftList;
    std::vector<int32_t> rightList;

    for (auto line : input | Split('\n')) {
        std::size_t space = line.find(' ');
        leftList.emplace_back(ParseNumber<int32_t>(line.substr(0, space)));
        rightList.emplace_back(ParseNumber<int32_t>(line.substr(space + 3)));
    }

    ranges::sort(leftList);
    ranges::sort(rightList);

    int64_t diff{};
    for (auto [lhs, rhs] : views::zip(leftList, rightList)) {
        diff += std::abs(rhs - lhs);
    }
    logger.solution("{}", diff);

    int64_t part2{};
    auto rightIt = rightList.begin();
    for (const int32_t left : leftList) {
        rightIt = ranges::find_if(rightIt, rightList.end(), [left](int32_t right) { return right >= left; });
        if (rightIt == rightList.end()) {
            break;
        }
        int32_t right = *rightIt;
        if (right > left) {
            continue;
        }
        auto greaterIt = ranges::find_if(rightIt, rightList.end(), [left](int32_t right) { return right > left; });
        part2 += right * (greaterIt - rightIt);
    }
    logger.solution("{}", part2);
}