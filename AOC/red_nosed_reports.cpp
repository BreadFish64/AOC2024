namespace {

auto Parse(std::string_view input) {
    return input | Split('\n') |
           views::transform([](std::string_view line) { return line | ParseNumbers<int32_t> | ranges::to_vector; }) |
           ranges::to_vector;
}

bool ReportIsSafe(auto&& report) {
    bool increasing = true;
    bool decreasing = true;
    bool gradual    = ranges::all_of(ranges::views::zip(report, report | ranges::views::drop(1)) |
                                         std::ranges::views::transform([&increasing, &decreasing](auto&& e) -> bool {
                                          auto [lhs, rhs] = e;
                                          increasing &= rhs > lhs;
                                          decreasing &= lhs > rhs;
                                          return std::abs(rhs - lhs) <= 3;
                                      }),
                                     std::identity{});
    return (increasing ^ decreasing) && gradual;
}

size_t CountSafe(auto&& reports) {
    return ranges::count_if(reports, [](std::span<int32_t> report) { return ReportIsSafe(report); });
}

size_t CountSafeWithMargin(auto&& reports) {
    return ranges::count_if(reports, [](std::span<int32_t> report) {
        if (ReportIsSafe(report)) {
            return true;
        }
        for (std::size_t discard = 0; discard < report.size(); ++discard) {
            if (ReportIsSafe(views::concat(report | views::take(discard), report | views::drop(discard + 1)))) {
                return true;
            }
        }
        return false;
    });
}

} // namespace

void AocMain(std::string_view input) {
    auto reports = StopWatch<std::micro>::Run("Parse", Parse, input);
    logger.solution("{}", StopWatch<std::micro>::Run("Part 1", [&reports] { return CountSafe(reports); }));
    logger.solution("{}", StopWatch<std::micro>::Run("Part 2", [&reports] { return CountSafeWithMargin(reports); }));
}
