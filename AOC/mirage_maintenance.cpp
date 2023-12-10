namespace {
Eigen::Array2i Extrapolate(std::span<int> history) {
    Eigen::Array2i edges{0, 0};
    int lmul = 1;
    while (!ranges::all_of(history, [](int entry) { return entry == 0; })) {
        edges += Eigen::Array2i{lmul * history.front(), history.back()};
        ranges::adjacent_difference(history, history.begin());
        history = history.subspan(1);
        lmul    = -lmul;
    }
    return edges;
}

auto Parse(std::string_view input) {
    return input | Split('\n') | views::transform([](std::string_view line) {
               return line | ParseNumbers<int> | ranges::to<boost::container::static_vector<int, 21>>;
           }) |
           ranges::to<std::vector>;
}
} // namespace

void AocMain(std::string_view input) {
    auto histories = StopWatch<std::milli>::Run("Parse", Parse, input);
    logger.solution("Solution: {}", StopWatch<std::micro>::Run("Solution", ranges::accumulate, histories,
                                                               Eigen::Array2i{0, 0}, std::plus{}, Extrapolate));
}