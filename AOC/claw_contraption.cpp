namespace {

[[maybe_unused]] constexpr std::string_view test = R"(Button A: X+94, Y+34
Button B: X+22, Y+67
Prize: X=8400, Y=5400

Button A: X+26, Y+66
Button B: X+67, Y+21
Prize: X=12748, Y=12176

Button A: X+17, Y+86
Button B: X+84, Y+37
Prize: X=7870, Y=6450

Button A: X+69, Y+23
Button B: X+27, Y+71
Prize: X=18641, Y=10279
)";

struct Machine {
    Eigen::Vector2<int64_t> a;
    Eigen::Vector2<int64_t> b;
    Eigen::Vector2<int64_t> prize;

    Machine(std::string_view machineString) {
        auto lines  = machineString | Split('\n');
        auto lineIt = lines.begin();
        auto fetch  = [](std::string_view line, std::string_view xSep, std::string_view ySep) {
            size_t x = line.find(xSep) + xSep.size();
            size_t y = line.find(ySep, x) + ySep.size();
            return Eigen::Vector2<int64_t>{
                ParseNumber<int64_t>(line.substr(x, line.find(',', x))),
                ParseNumber<int64_t>(line.substr(y)),
            };
        };
        a     = fetch(*lineIt++, "X+", "Y+");
        b     = fetch(*lineIt++, "X+", "Y+");
        prize = fetch(*lineIt++, "X=", "Y=");
    }

    // a0x + b0y = p0
    // a1x + b1y = p1
    //
    // x = (p0 - b0y) / a0
    // x = (p1 - b1y) / a1
    //
    // (p0 - b0y) / a0 = (p1 - b1y) / a1
    // a1(p0 - b0y) = a0(p1 - b1y)
    // p0 - a1b0y = a0p1 - a0b1y
    // a0b1y - a1b0y = a0p1 - a1p0
    // y(a0b1 - a1b0) = a0p1 - a1p0
    // y = (a0p1 - a1p0) / (a0b1 - a1b0)
    int64_t minimumPrizeCost(Eigen::Vector2<int64_t> offset) const {
        const Eigen::Vector2<int64_t> p{prize + offset};
        auto [y, yRem] = std::div(a[0] * p[1] - a[1] * p[0], a[0] * b[1] - a[1] * b[0]);
        if (yRem) return 0;
        auto [x, xRem] = std::div(p[0] - b[0] * y, a[0]);
        if (xRem) return 0;
        return 3 * x + y;
    }

    int64_t part1() const { return minimumPrizeCost({0, 0}); }
    int64_t part2() const { return minimumPrizeCost({10000000000000, 10000000000000}); }
};

} // namespace

void AocMain(std::string_view input) {
    // input                         = test;
    std::vector<Machine> machines = StopWatch<std::micro>::Run("Parsing", [&] {
        return input | Split("\n\n"sv) | views::transform(Constructor<Machine>{}) | ranges::to_vector;
    });

    logger.solution("Part 1: {}", StopWatch<std::micro>::Run("Part 1", [&] {
                        return ranges::accumulate(machines, int64_t{}, std::plus{}, &Machine::part1);
                    }));
    logger.solution("Part 2: {}", StopWatch<std::micro>::Run("Part 2", [&] {
                        return ranges::accumulate(machines, int64_t{}, std::plus{}, &Machine::part2);
                    }));

    std::vector<double> a0, a1, b0, b1, p0, p1;
    const auto deinterleave = [&machines](std::vector<double>& column, auto&& proj) {
        column.resize(machines.size());
        ranges::transform(machines, column.begin(), proj);
    };
    deinterleave(a0, [](const Machine& m) { return m.a[0]; });
    deinterleave(a1, [](const Machine& m) { return m.a[1]; });
    deinterleave(b0, [](const Machine& m) { return m.b[0]; });
    deinterleave(b1, [](const Machine& m) { return m.b[1]; });
    deinterleave(p0, [](const Machine& m) { return m.prize[0]; });
    deinterleave(p1, [](const Machine& m) { return m.prize[1]; });

    int64_t offset = 0;
    __m256d voffset{_mm256_set1_pd(0.0)};
    auto doSimd = [&] {
        __m256d vacc = _mm256_set1_pd(0.0);
        size_t i{0};
        for (; i + 4 <= machines.size(); i += 4) {
            __m256d vp0 = _mm256_add_pd(voffset, _mm256_loadu_pd(&p0[i]));
            __m256d vp1 = _mm256_add_pd(voffset, _mm256_loadu_pd(&p1[i]));
            __m256d va0 = _mm256_loadu_pd(&a0[i]);
            __m256d va1 = _mm256_loadu_pd(&a1[i]);
            __m256d vb0 = _mm256_loadu_pd(&b0[i]);
            __m256d vb1 = _mm256_loadu_pd(&b1[i]);

            __m256d yNum = _mm256_fmsub_pd(va0, vp1, _mm256_mul_pd(va1, vp0));
            __m256d yDen = _mm256_fmsub_pd(va0, vb1, _mm256_mul_pd(va1, vb0));
            __m256d y    = _mm256_div_pd(yNum, yDen);
            __m256d x    = _mm256_div_pd(_mm256_fnmadd_pd(vb0, y, vp0), va0);

            __m256d yMask = _mm256_cmp_pd(y, _mm256_floor_pd(y), _CMP_EQ_OQ);
            __m256d xMask = _mm256_cmp_pd(x, _mm256_floor_pd(x), _CMP_EQ_OQ);

                        __m256d vcost = _mm256_fmadd_pd(x, _mm256_set1_pd(3.0), y);
            vacc          = _mm256_add_pd(vacc, _mm256_and_pd(vcost, _mm256_and_pd(xMask, yMask)));
        }

        vacc        = _mm256_hadd_pd(vacc, _mm256_setzero_pd());
        int64_t acc = static_cast<int64_t>(_mm256_cvtsd_f64(vacc) + _mm_cvtsd_f64(_mm256_extractf128_pd(vacc, 1)));

        for (; i < machines.size(); ++i) {
            acc += machines[i].minimumPrizeCost({offset, offset});
        }
        return acc;
    };
    logger.solution("Part 1 SIMD: {}", StopWatch<std::micro>::Run("Part 1 SIMD", doSimd));
    offset  = 10000000000000;
    voffset = _mm256_set1_pd(10000000000000);
    logger.solution("Part 2 SIMD: {}", StopWatch<std::micro>::Run("Part 2 SIMD", doSimd));
}