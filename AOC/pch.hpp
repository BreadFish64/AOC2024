#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <bitset>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <execution>
#include <fstream>
#include <future>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <ranges>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_set>
#include <utility>

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>

#include <Eigen/Dense>

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include <fmt/ostream.h>

#include <experimental/mdspan>

#include <range/v3/all.hpp>

#include <mio/mmap.hpp>

#include <immintrin.h>

namespace views = ranges::views;

using namespace std::literals;

using u8   = std::uint8_t;
using u16  = std::uint16_t;
using u32  = std::uint32_t;
using u64  = std::uint64_t;
using u128 = boost::multiprecision::uint128_t;

using s8   = std::int8_t;
using s16  = std::int16_t;
using s32  = std::int32_t;
using s64  = std::int64_t;
using s128 = boost::multiprecision::int128_t;

using f32 = float;
using f64 = double;

using usize = std::size_t;
using ssize = std::make_signed_t<usize>;

constexpr s64 operator""_s64(unsigned long long x) {
    return static_cast<s64>(x);
}

constexpr u64 operator""_u64(unsigned long long x) {
    return static_cast<u64>(x);
}

constexpr usize operator""_sz(unsigned long long x) {
    return static_cast<usize>(x);
}

template <typename I>
constexpr I AlignUp(I x, I alignment) {
    x = x + alignment - 1;
    return x - (x % alignment);
}

template <typename I>
constexpr I DivCeil(I x, I y) {
    return (x + y - 1) / y;
}

template <typename I, typename P>
constexpr I Power(I x, P y) {
    I result = 1;
    while (y != 0) {
        if (y & 1) result *= x;
        x *= x;
        y >>= 1;
    }
    return result;
}

template <typename T>
T sign(T val) {
    return (T(0) < val) - (val < T(0));
}

inline std::tuple<double, double> SolveQuadratic(double a, double b, double c) {
    double radical = std::sqrt(b * b - 4 * a * c);
    double r2a     = 1.0 / (2.0 * a);
    double lower   = (-b - radical) * r2a;
    double upper   = (-b + radical) * r2a;
    if (lower > upper) std::swap(lower, upper);
    return {lower, upper};
}

inline std::bitset<8> rotr(std::bitset<8> x, int s) {
    return std::rotr(static_cast<u8>(x.to_ulong()), s);
}
inline std::bitset<8> rotl(std::bitset<8> x, int s) {
    return std::rotl(static_cast<u8>(x.to_ulong()), s);
}
inline std::bitset<16> rotr(std::bitset<16> x, int s) {
    return std::rotr(static_cast<u16>(x.to_ulong()), s);
}
inline std::bitset<16> rotl(std::bitset<16> x, int s) {
    return std::rotl(static_cast<u16>(x.to_ulong()), s);
}
inline std::bitset<32> rotr(std::bitset<32> x, int s) {
    return std::rotr(static_cast<u32>(x.to_ulong()), s);
}
inline std::bitset<32> rotl(std::bitset<32> x, int s) {
    return std::rotl(static_cast<u32>(x.to_ulong()), s);
}
inline std::bitset<64> rotr(std::bitset<64> x, int s) {
    return std::rotr(static_cast<u64>(x.to_ullong()), s);
}
inline std::bitset<64> rotl(std::bitset<64> x, int s) {
    return std::rotl(static_cast<u64>(x.to_ullong()), s);
}

constexpr bool IsGraph(char c) {
    return c >= 33 && c <= 126;
}

constexpr bool IsNotGraph(char c) {
    return !IsGraph(c);
}

constexpr bool IsDigit(char c) {
    return c >= (int)'0' && c <= (int)'9';
}

constexpr bool IsNotDigit(char c) {
    return !IsDigit(c);
}

constexpr bool IsAlpha(char c) {
    return (c >= (int)'A' && c <= (int)'Z') || (c >= (int)'a' && c <= (int)'z');
}

constexpr bool IsAlNum(char c) {
    return IsDigit(c) || IsAlpha(c);
}

constexpr bool IsPunct(char c) {
    return IsGraph(c) && !IsAlNum(c);
}

constexpr auto ChunkToStringView = views::transform([](auto&& chunk) {
    return std::string_view{&*ranges::begin(chunk), static_cast<std::size_t>(ranges::distance(chunk))};
});

constexpr auto SplitWs =
    views::drop_while(IsNotGraph) |
    views::split_when([](ranges::bidirectional_iterator auto begin, ranges::bidirectional_iterator auto end) {
        auto whitespace_end = std::find_if(begin, end, IsGraph);
        return std::make_pair(begin != whitespace_end, whitespace_end);
    }) |
    ChunkToStringView;

constexpr auto Split(const auto& c) {
    return views::split(c) | ChunkToStringView;
}

inline void ThrowErrc(std::errc errc) {
    throw std::runtime_error{std::make_error_condition(errc).message()};
}

template <typename T>
inline T ParseNumber(std::string_view str) {
    T val{};
    if (str.front() == '+') str.remove_prefix(1);
    if (const std::from_chars_result result = std::from_chars(str.data(), str.data() + str.size(), val);
        result.ec != std::errc{}) [[unlikely]] {
        ThrowErrc(result.ec);
    }
    return val;
}

template <typename T>
constexpr auto ParseNumbers = SplitWs | views::transform(ParseNumber<T>);

class Logger {
    using TypeErasedLogLine = std::move_only_function<void() const>;

    template <typename... Args>
    struct LogLine {
        fmt::text_style storedStyle;
        fmt::format_string<Args...> storedFmt;
        std::tuple<Args...> storedArgs;

        explicit LogLine(const fmt::text_style& style, fmt::format_string<Args...> fmt, const Args&... args)
            : storedStyle{style}, storedFmt{fmt}, storedArgs{std::make_tuple(args...)} {}

        void operator()() const {
            auto printArgs = [this](const Args&... unpackedArgs) {
                fmt::print(stdout, storedStyle, storedFmt.get(), unpackedArgs...);
            };
            std::apply(printArgs, storedArgs);
            std::putc('\n', stdout);
        }
    };
    std::mutex logLinesMutex;
    std::vector<TypeErasedLogLine> logLines;

    template <typename... Args>
    void addLogline(const fmt::text_style& style, fmt::format_string<Args...> fmt, const Args&... args) {
        const std::lock_guard logLinesLock{logLinesMutex};
        logLines.emplace_back(std::in_place_type<LogLine<Args...>>, style, fmt, args...);
    }

public:
    Logger() { logLines.reserve(128); }

    void flush() {
        std::vector<TypeErasedLogLine> flushedLines;
        {
            const std::lock_guard logLinesLock{logLinesMutex};
            logLines.swap(flushedLines);
        }
        for (const auto& logLine : flushedLines) {
            logLine();
        }
    }

    template <typename... Args>
    void info(fmt::format_string<Args...> fmt, const Args&... args) {
        addLogline({}, fmt, args...);
    }

    template <typename... Args>
    void perf(fmt::format_string<Args...> fmt, const Args&... args) {
        addLogline(fmt::fg(fmt::color::cyan), fmt, args...);
    }

    template <typename... Args>
    void solution(fmt::format_string<Args...> fmt, const Args&... args) {
        addLogline(fmt::fg(fmt::color::lime), fmt, args...);
    }
};

inline Logger logger;

template <typename Ratio = std::milli>
struct StopWatch {
    const std::string sectionName;
    const std::chrono::steady_clock::time_point start;

    explicit StopWatch(std::string sectionName)
        : sectionName{std::move(sectionName)}, start{std::chrono::steady_clock::now()} {}

    ~StopWatch() {
        const auto stop = std::chrono::steady_clock::now();
        const std::chrono::duration<double, Ratio> duration{stop - start};
        logger.perf("{} took {}", sectionName, duration);
    }

    template <typename... Args, std::invocable<Args...> Function>
    static auto Run(std::string sectionName, Function&& function, Args&&... args) {
        StopWatch runStopWatch{std::move(sectionName)};
        return std::invoke(std::forward<Function>(function), std::forward<Args>(args)...);
    }
};

inline const std::string INPUT_FILE_NAME{"input.txt"s};

inline auto LoadInput() {
    return mio::mmap_source{INPUT_FILE_NAME};
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

void AocMain(std::string_view input);

int main(const int argc, const char* argv[]) {
    {
        StopWatch wholeProgramStopWatch{"Whole Program"};
        SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
        SetConsoleCP(CP_UTF8);
        SetConsoleOutputCP(CP_UTF8);
        const auto input = StopWatch<std::micro>::Run("LoadInput", LoadInput);
        StopWatch<std::milli>::Run("AocMain", AocMain, std::string_view{input});
    }
    logger.flush();
}