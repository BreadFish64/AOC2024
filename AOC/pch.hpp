#pragma once

#include <immintrin.h>

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

#include <experimental/mdspan>

#include <range/v3/all.hpp>

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>

#include <Eigen/Dense>

#include <mio/mmap.hpp>

#include <lodepng.h>

#include "tsc.hpp"

using namespace std::literals;
namespace views = ranges::views;

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
#if defined(__GNUC__) || defined(__clang__)
using u128 = unsigned __int128;
#else
using u128 = boost::multiprecision::uint128_t;
#endif

using s8  = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;
#ifdef __GNUC__ || defined(__clang__)
using s128 = signed __int128;
#else
using s128 = boost::multiprecision::int128_t;
#endif

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

template <std::floating_point F = double>
inline std::tuple<F, F> SolveQuadratic(F a, F b, F c) {
    F radical = std::sqrt(b * b - 4 * a * c);
    F r2a     = 1.0 / (2.0 * a);
    F lower   = (-b - radical) * r2a;
    F upper   = (-b + radical) * r2a;
    if (lower > upper) std::swap(lower, upper);
    return {lower, upper};
}

template <std::integral I>
constexpr I ModularMultiplicativeInverse(I a, I m) {
    const I m0 = m;
    I x0 = 0, x1 = 1;
    if (m0 == 1) return 0;
    while (a > 1) {
        I q = a / m;
        a   = std::exchange(m, a % m);
        x1  = std::exchange(x0, x1 - q * x0);
    }
    if (x1 < 0) x1 += m0;
    return x1;
}

template <std::integral I>
constexpr I ChineseRemainderTheorem(std::span<const I> modulos, std::span<const I> remainders) {
    const I product = ranges::accumulate(modulos, I{1}, std::multiplies{});
    I result        = ranges::accumulate(views::zip(modulos, remainders), I{}, std::plus{}, [product](const auto& it) {
        const auto [modulo, remainder] = it;
        I pp                           = product / modulo;
        return remainder * ModularMultiplicativeInverse(pp, modulo) * pp;
    });
    return result % product;
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

[[gnu::always_inline]] inline u32 popcount(u128 i) {
    return std::popcount(static_cast<u64>(i & 0xFFFFFFFFFFFFFFFFULL)) + std::popcount(static_cast<u64>(i >> 64));
}
[[gnu::always_inline]] inline u32 countr_zero(u128 i) {
    const auto lo = static_cast<u64>(i & 0xFFFFFFFFFFFFFFFFULL);
    const auto hi = static_cast<u64>(i >> 64);
    return (lo == 0) ? std::countr_zero(hi) + 64 : std::countr_zero(lo);
}
[[gnu::always_inline]] inline u32 countr_one(u128 i) {
    const auto lo = static_cast<u64>(i & 0xFFFFFFFFFFFFFFFFULL);
    const auto hi = static_cast<u64>(i >> 64);
    return (~lo == 0) ? std::countr_one(hi) + 64 : std::countr_one(lo);
}
[[gnu::always_inline]] inline u32 countl_zero(u128 i) {
    const auto lo = static_cast<u64>(i & 0xFFFFFFFFFFFFFFFFULL);
    const auto hi = static_cast<u64>(i >> 64);
    return (hi == 0) ? std::countl_zero(lo) + 64 : std::countl_zero(hi);
}

[[gnu::always_inline]] inline u128 pdep(u128 src, u128 mask) {
    u64 low  = _pdep_u64(static_cast<u64>(src), static_cast<u64>(mask & 0xFFFFFFFFFFFFFFFFULL));
    u64 high = _pdep_u64(static_cast<u64>(src >> std::popcount(static_cast<u64>(mask & 0xFFFFFFFFFFFFFFFFULL))),
                         static_cast<u64>(mask >> 64));
    return u128{high} << 64 | low;
}

// https://stackoverflow.com/a/8281965
template <typename I>
[[gnu::always_inline]] inline I bit_twiddle_permute(I v) {
    I t = v | (v - 1); // t gets v's least significant 0 bits set to 1
    // Next set to 1 the most significant bit to change,
    // set to 0 the least significant ones, and add the necessary 1 bits.
    I w = (t + 1) | (((~t & -~t) - 1) >> (countr_zero(v) + 1));
    return w;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// https://github.com/yuzu-emu/yuzu/blob/master/src/common/container_hash.h
template <std::unsigned_integral T>
constexpr std::size_t HashValue(T val) {
    const unsigned int size_t_bits = std::numeric_limits<std::size_t>::digits;
    const unsigned int length      = (std::numeric_limits<T>::digits - 1) / static_cast<unsigned int>(size_t_bits);

    std::size_t seed = 0;

    for (unsigned int i = length * size_t_bits; i > 0; i -= size_t_bits) {
        seed ^= static_cast<size_t>(val >> i) + (seed << 6) + (seed >> 2);
    }

    seed ^= static_cast<size_t>(val) + (seed << 6) + (seed >> 2);

    return seed;
}

constexpr std::size_t HashValue(const u128& val);

template <size_t Bits>
struct HashCombineImpl {
    template <typename T>
    static constexpr T fn(T seed, T value) {
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

template <>
struct HashCombineImpl<64> {
    static constexpr std::uint64_t fn(std::uint64_t h, std::uint64_t k) {
        const std::uint64_t m = (std::uint64_t(0xc6a4a793) << 32) + 0x5bd1e995;
        const int r           = 47;

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;

        // Completely arbitrary number, to prevent 0's
        // from hashing to 0.
        h += 0xe6546b64;

        return h;
    }
};

template <typename T>
constexpr void HashCombine(std::size_t& seed, const T& v) {
    seed = HashCombineImpl<sizeof(std::size_t) * CHAR_BIT>::fn(seed, HashValue(v));
}

constexpr std::size_t HashValue(const u128& val) {
    size_t seed = 0;
    HashCombine(seed, HashValue(static_cast<u64>(val)));
    HashCombine(seed, HashValue(static_cast<u64>(val >> 64)));
    return seed;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr bool IsGraph(char c) {
    return c >= 33 && c <= 126;
}

constexpr bool IsNotGraph(char c) {
    return !IsGraph(c);
}

constexpr bool IsDigit(char c) {
    return c >= '0' && c <= '9';
}

constexpr bool IsNotDigit(char c) {
    return !IsDigit(c);
}

constexpr bool IsAlpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
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

// template <typename Fun>
// constexpr auto ApplyView(Fun fun) {
//     return views::transform([mutable fun = std::move(fun)](T&& args) < typename T >
//                             { return std::apply(fun, std::forward<T>(args)); });
// }

[[msvc::noinline]] inline void ThrowError(const std::errc errc) {
    throw std::runtime_error{std::make_error_condition(errc).message()};
}

[[msvc::forceinline]] inline void ThrowOnError(const std::errc errc) {
    if (errc != std::errc{}) [[unlikely]] {
        ThrowError(errc);
    }
}

[[msvc::forceinline]] inline void ThrowOnError(const std::from_chars_result& result) {
    ThrowOnError(result.ec);
}

template <typename T>
inline T ParseNumber(std::string_view str) {
    T val{};
    if (str.front() == '+') str.remove_prefix(1);
    ThrowOnError(std::from_chars(str.data(), str.data() + str.size(), val));
    return val;
}

#pragma region tuple_like_Eigen_Array

template <typename T, int N>
struct std::tuple_size<Eigen::Array<T, N, 1, 0, N, 1>> : public integral_constant<std::size_t, N> {};

template <std::size_t I, typename T, int N>
struct std::tuple_element<I, Eigen::Array<T, N, 1, 0, N, 1>> {
    using type = T;
};

template <std::size_t I, typename T, int N>
    requires(I < N)
inline const T& get(const Eigen::Array<T, N, 1, 0, N, 1>& arr) {
    return arr[I];
};

template <std::size_t I, typename T, int N>
    requires(I < N)
inline T& get(Eigen::Array<T, N, 1, 0, N, 1>& arr) {
    return arr[I];
};

template <typename T, int N>
inline std::span<const T, N> ToSpan(const Eigen::Array<T, N, 1, 0, N, 1>& arr) {
    return std::span<const T, N>{arr.data(), static_cast<size_t>(arr.size())};
}

template <typename T, int N>
inline std::span<T, N> ToSpan(Eigen::Array<T, N, 1, 0, N, 1>& arr) {
    return std::span<T, N>{arr.data(), static_cast<size_t>(arr.size())};
}

#pragma endregion

template <typename T>
constexpr auto ParseNumbers = SplitWs | views::transform(ParseNumber<T>);

template <typename T>
concept TupleLike = requires {
    { std::tuple_size_v<T> } -> std::convertible_to<size_t>;
};

class Logger {
    using TypeErasedLogLine = std::move_only_function<void() const>;

    template <typename T>
    static auto CopyRange(T&& obj) {
        using Value = std::remove_cvref_t<T>;
        if constexpr (std::convertible_to<T, std::string>) {
            return std::string(std::forward<T>(obj));
        } else if constexpr (ranges::range<T>) {
            using Element = ranges::value_type_t<T>;
            if constexpr (std::same_as<char, Element>) {
                return std::string(ranges::begin(obj), ranges::end(obj));
            } /*else if constexpr (TupleLike<Value>) {
                return boost::container::static_vector<Element, std::tuple_size_v<Value>>(ranges::begin(obj),
                                                                                          ranges::end(obj));
            }*/
            else {
                return boost::container::small_vector<Element, 256 / sizeof(Element)>(ranges::begin(obj),
                                                                                      ranges::end(obj));
            }
        } else {
            return std::forward<T>(obj);
        }
    }

    template <typename... Args>
    struct LogLine {
        fmt::text_style storedStyle;
        fmt::string_view storedFmt;
        std::tuple<decltype(CopyRange(std::declval<Args>()))...> storedArgs;

        explicit LogLine(const fmt::text_style& style, fmt::format_string<Args...> fmt, Args&&... args)
            : storedStyle{style}, storedFmt{fmt.get()},
              storedArgs{std::make_tuple(CopyRange(std::forward<Args>(args))...)} {}

        void operator()() const {
            auto printArgs = [this](const auto&... unpackedArgs) {
                fmt::vprint(stdout, storedStyle, storedFmt, fmt::make_format_args(unpackedArgs...));
            };
            std::apply(printArgs, storedArgs);
            std::putc('\n', stdout);
        }
    };
    std::mutex logLinesMutex;
    std::vector<TypeErasedLogLine> logLines;

    template <typename... Args>
    void addLogline(const fmt::text_style& style, fmt::format_string<Args...> fmt, Args&&... args) {
        const std::lock_guard logLinesLock{logLinesMutex};
        logLines.emplace_back(std::in_place_type<LogLine<Args...>>, style, fmt, std::forward<Args>(args)...);
    }

public:
    explicit Logger();

    void flush();

    template <typename... Args>
    void info(fmt::format_string<Args...> fmt, Args&&... args) {
        addLogline({}, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void perf(fmt::format_string<Args...> fmt, Args&&... args) {
        addLogline(fmt::fg(fmt::color::cyan), fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void solution(fmt::format_string<Args...> fmt, Args&&... args) {
        addLogline(fmt::fg(fmt::color::lime), fmt, std::forward<Args>(args)...);
    }
};

extern Logger logger;

template <typename Ratio = std::milli>
struct StopWatch {
    const std::string sectionName;
    const uint64_t startTick;

    explicit StopWatch(std::string sectionName) : sectionName{std::move(sectionName)}, startTick{TSC::GetTicks()} {}

    ~StopWatch() {
        const auto stopTick = TSC::GetTicks();
        const auto duration = TSC::TicksTo<std::chrono::duration<double, Ratio>>(stopTick - startTick);
        logger.perf("{} took {}", sectionName, duration);
    }

    template <typename... Args, std::invocable<Args...> Function>
    static auto Run(std::string sectionName, Function&& function, Args&&... args) {
        StopWatch runStopWatch{std::move(sectionName)};
        return std::invoke(std::forward<Function>(function), std::forward<Args>(args)...);
    }
};

#ifdef NDEBUG
#define UNREACHABLE() __assume(false)
#define ASSUME(x) __assume(x)
#else
#define UNREACHABLE() assert(false)
#define ASSUME(x) assert(x)
#endif

constexpr void Assume(bool x) {
    if (std::is_constant_evaluated()) {
        if (!x) {
            throw std::invalid_argument{"Assumption failed"};
        }
    } else {
        ASSUME(x);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

void AocMain(std::string_view input);
