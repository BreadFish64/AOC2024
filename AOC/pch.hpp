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
#include <format>
#include <future>
#include <iterator>
#include <map>
#include <memory>
#include <memory_resource>
#include <numeric>
#include <optional>
#include <ranges>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <unordered_set>
#include <utility>

#include <range/v3/all.hpp>

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>

#include "Attr.hpp"
#include "Fnv.hpp"
#include "Mdspan.hpp"
#include "TscClock.hpp"

#undef IN

using namespace std::literals;
namespace views = ranges::views;

#if defined(__GNUC__) || defined(__clang__)
using int128_t  = signed __int128;
using uint128_t = unsigned __int128;
#else
using int128_t  = boost::multiprecision::int128_t;
using uint128_t = boost::multiprecision::uint128_t;
#endif

constexpr std::int64_t operator""_int64_t(unsigned long long x) {
    return static_cast<std::int64_t>(x);
}

constexpr std::uint64_t operator""_uint64_t(unsigned long long x) {
    return static_cast<std::uint64_t>(x);
}

constexpr std::size_t operator""_size_t(unsigned long long x) {
    return static_cast<std::size_t>(x);
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
constexpr I Power(I base, P exp) {
    I result = 1;
    while (exp != 0) {
        if (exp & 1) {
            result *= base;
        }
        base *= base;
        exp >>= 1;
    }
    return result;
}

template <auto base, class T>
constexpr unsigned LogPlusOne(T x) {
    unsigned log{};
    T power{1};
    while (power <= x) {
        ++log;
        power *= base;
    }
    return log;
}

template <typename T>
T Sign(T val) {
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

template <std::unsigned_integral U>
constexpr U BitreversePortable(U x) noexcept {
    x = std::byteswap(x);

    const auto Extract = [x](int i) -> U {
        const U mask{static_cast<U>(0x0101010101010101ULL) << i};
        return x & mask;
    };

    // Eliminating dependencies allows clang to vectorize this for std::uint32_t and u16
    U x0{Extract(0) << 7};
    U x1{Extract(1) << 5};
    U x2{Extract(2) << 3};
    U x3{Extract(3) << 1};

    U x4{Extract(4) >> 1};
    U x5{Extract(5) >> 3};
    U x6{Extract(6) >> 5};
    U x7{Extract(7) >> 7};

    x0 |= x4;
    x1 |= x5;
    x2 |= x6;
    x3 |= x7;

    x0 |= x2;
    x1 |= x3;

    x = x0 | x1;

    return x;
}

template <std::unsigned_integral U>
[[attr_flatten]] constexpr U Bitreverse(U x) noexcept {
    return BitreversePortable<U>(x);
}

template <>
[[attr_flatten]] constexpr std::uint32_t Bitreverse<std::uint32_t>(std::uint32_t x) noexcept {
    if (std::is_constant_evaluated()) {
        return BitreversePortable<std::uint32_t>(x);
    } else {
        x              = std::byteswap(x);
        __m128i x_32x4 = _mm_set1_epi32(x);
        const __m128i slMask_32x4 =
            _mm_set_epi32(0x01010101ULL << 0, 0x01010101ULL << 1, 0x01010101ULL << 2, 0x01010101ULL << 3);
        const __m128i srMask_32x4 =
            _mm_set_epi32(0x01010101ULL << 7, 0x01010101ULL << 6, 0x01010101ULL << 4, 0x01010101ULL << 4);
        const __m128i shift_32x4   = _mm_set_epi32(7, 5, 3, 1);
        const __m128i xl_32x4      = _mm_sllv_epi32(_mm_and_si128(x_32x4, slMask_32x4), shift_32x4);
        const __m128i xr_32x4      = _mm_srlv_epi32(_mm_and_si128(x_32x4, srMask_32x4), shift_32x4);
        x_32x4                     = _mm_or_si128(xl_32x4, xr_32x4);
        const std::uint64_t x_32x2 = _mm_extract_epi64(x_32x4, 0) | _mm_extract_epi64(x_32x4, 0);
        return static_cast<std::uint32_t>(x_32x2) | static_cast<std::uint32_t>(x_32x2 >> 32);
    }
}

template <>
[[attr_flatten]] constexpr std::uint64_t Bitreverse<std::uint64_t>(std::uint64_t x) noexcept {
    if (std::is_constant_evaluated()) {
        return BitreversePortable<std::uint64_t>(x);
    } else {
        x                         = std::byteswap(x);
        __m256i x_64x4            = _mm256_set1_epi64x(x);
        const __m256i slMask_64x4 = _mm256_set_epi64x(0x0101010101010101ULL << 0, 0x0101010101010101ULL << 1,
                                                      0x0101010101010101ULL << 2, 0x0101010101010101ULL << 3);
        const __m256i srMask_64x4 = _mm256_set_epi64x(0x0101010101010101ULL << 7, 0x0101010101010101ULL << 6,
                                                      0x0101010101010101ULL << 4, 0x0101010101010101ULL << 4);
        const __m256i shift_64x4  = _mm256_set_epi64x(7, 5, 3, 1);
        const __m256i xl_64x4     = _mm256_sllv_epi64(_mm256_and_si256(x_64x4, slMask_64x4), shift_64x4);
        const __m256i xr_64x4     = _mm256_srlv_epi64(_mm256_and_si256(x_64x4, srMask_64x4), shift_64x4);
        x_64x4                    = _mm256_or_si256(xl_64x4, xr_64x4);
        __m128i x_64x2 = _mm_or_si128(_mm256_extracti128_si256(x_64x4, 0), _mm256_extracti128_si256(x_64x4, 1));
        return _mm_extract_epi64(x_64x2, 0) | _mm_extract_epi64(x_64x2, 1);
    }
}

[[gnu::always_inline]] inline std::uint32_t popcount(uint128_t i) {
    return std::popcount(static_cast<std::uint64_t>(i & 0xFFFFFFFFFFFFFFFFULL)) +
           std::popcount(static_cast<std::uint64_t>(i >> 64));
}
[[gnu::always_inline]] inline std::uint32_t countr_zero(uint128_t i) {
    const auto lo = static_cast<std::uint64_t>(i & 0xFFFFFFFFFFFFFFFFULL);
    const auto hi = static_cast<std::uint64_t>(i >> 64);
    return (lo == 0) ? std::countr_zero(hi) + 64 : std::countr_zero(lo);
}
[[gnu::always_inline]] inline std::uint32_t countr_one(uint128_t i) {
    const auto lo = static_cast<std::uint64_t>(i & 0xFFFFFFFFFFFFFFFFULL);
    const auto hi = static_cast<std::uint64_t>(i >> 64);
    return (~lo == 0) ? std::countr_one(hi) + 64 : std::countr_one(lo);
}
[[gnu::always_inline]] inline std::uint32_t countl_zero(uint128_t i) {
    const auto lo = static_cast<std::uint64_t>(i & 0xFFFFFFFFFFFFFFFFULL);
    const auto hi = static_cast<std::uint64_t>(i >> 64);
    return (hi == 0) ? std::countl_zero(lo) + 64 : std::countl_zero(hi);
}

[[gnu::always_inline]] inline uint128_t pdep(uint128_t src, uint128_t mask) {
    std::uint64_t low =
        _pdep_u64(static_cast<std::uint64_t>(src), static_cast<std::uint64_t>(mask & 0xFFFFFFFFFFFFFFFFULL));
    std::uint64_t high = _pdep_u64(
        static_cast<std::uint64_t>(src >> std::popcount(static_cast<std::uint64_t>(mask & 0xFFFFFFFFFFFFFFFFULL))),
        static_cast<std::uint64_t>(mask >> 64));
    return uint128_t{high} << 64 | low;
}

// https://stackoverflow.com/a/8281965
template <typename I>
[[gnu::always_inline]] inline I BitTwiddlePermute(I v) {
    using std::countr_zero;
    I t = v | (v - 1); // t gets v's least significant 0 bits set to 1
    // Next set to 1 the most significant bit to change,
    // set to 0 the least significant ones, and add the necessary 1 bits.
    I w = (t + 1) | (((~t & -~t) - 1) >> (countr_zero(v) + 1));
    return w;
}

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

constexpr std::size_t HashValue(const uint128_t& val);

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

constexpr std::size_t HashValue(const uint128_t& val) {
    size_t seed = 0;
    HashCombine(seed, HashValue(static_cast<std::uint64_t>(val)));
    HashCombine(seed, HashValue(static_cast<std::uint64_t>(val >> 64)));
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

constexpr auto AsPointers = std::views::transform([](auto& ref) { return std::addressof(ref); });

[[attr_noinline]]
inline void ThrowError(const std::errc errc) {
    throw std::system_error{std::make_error_code(errc)};
}

[[attr_forceinline]] inline void ThrowOnError(const std::errc errc) {
    if (errc != std::errc{}) [[unlikely]] {
        ThrowError(errc);
    }
}

[[attr_forceinline]] inline void ThrowOnError(const std::from_chars_result& result) {
    ThrowOnError(result.ec);
}

template <typename T, int base = 10>
inline T ParseNumber(std::string_view str) {
    T val{};
    if (str.front() == '+') str.remove_prefix(1);
    ThrowOnError(std::from_chars(str.data(), str.data() + str.size(), val, base));
    return val;
}

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
        std::string storedStyle;
        std::string_view storedFmt;
        std::tuple<decltype(CopyRange(std::declval<Args>()))...> storedArgs;

        explicit LogLine(std::string style, std::format_string<Args...> fmt, Args&&... args)
            : storedStyle{std::move(style)}, storedFmt{fmt.get()},
              storedArgs{std::make_tuple(CopyRange(std::forward<Args>(args))...)} {}

        void operator()() const {
            std::fwrite(storedStyle.data(), sizeof(char), storedStyle.size(), stdout);
            auto printArgs = [this](const auto&... unpackedArgs) {
                std::vformat_to(std::ostreambuf_iterator{std::cout}, storedFmt, std::make_format_args(unpackedArgs...));
            };
            std::apply(printArgs, storedArgs);
            constexpr std::string_view resetAndNewline{"\x1b[0m\n"};
            std::fwrite(resetAndNewline.data(), sizeof(char), resetAndNewline.size(), stdout);
        }
    };
    std::mutex logLinesMutex;
    std::vector<TypeErasedLogLine> logLines;

    template <typename... Args>
    void addLogline(std::string style, std::format_string<Args...> fmt, Args&&... args) {
        const std::lock_guard logLinesLock{logLinesMutex};
        logLines.emplace_back(std::in_place_type<LogLine<Args...>>, std::move(style), fmt, std::forward<Args>(args)...);
    }

public:
    explicit Logger();

    void flush();

    template <typename... Args>
    void info(std::format_string<Args...> fmt, Args&&... args) {
        addLogline("", fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void perf(std::format_string<Args...> fmt, Args&&... args) {
        addLogline("\x1b[36m", fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void solution(std::format_string<Args...> fmt, Args&&... args) {
        addLogline("\x1b[92m", fmt, std::forward<Args>(args)...);
    }
};

extern Logger logger;

template <typename Ratio = std::milli>
struct StopWatch {
#ifdef WIN32
    using Clock = std::chrono::steady_clock;
#else
    using Clock = std::chrono::steady_clock;
#endif
    const std::string sectionName;
    const Clock::time_point start;

    explicit StopWatch(std::string sectionName) : sectionName{std::move(sectionName)}, start{Clock::now()} {}

    ~StopWatch() {
        const Clock::time_point stop = Clock::now();
        logger.perf("{} took {}", sectionName, std::chrono::duration<double, Ratio>(stop - start));
    }

    template <typename... Args, std::invocable<Args...> Function>
    static auto Run(std::string sectionName, Function&& function, Args&&... args) {
        StopWatch runStopWatch{std::move(sectionName)};
        return std::invoke(std::forward<Function>(function), std::forward<Args>(args)...);
    }
};

template <typename T>
struct Constructor {
    template <typename... Args>
    T operator()(Args&&... args) const {
        return T{std::forward<Args>(args)...};
    }
};

template <TupleLike T>
auto ToExtent(const T& tuple) {
    using Extents = dextents<std::remove_cvref_t<std::tuple_element<0, T>>, std::tuple_size_v<T>>;
    return std::make_from_tuple<Extents>(tuple);
}

template <std::weakly_incrementable... I>
constexpr auto MultiFor(I... ends) {
    return ranges::views::cartesian_product(ranges::views::iota(I{}, ends)...);
}
template <typename R, typename T>
constexpr auto PointerProject(R T::*member) {
    return [member](T* p) {
        return std::invoke(member, *p);
    };
}

template <typename R, typename T>
constexpr auto PointerProject(R (T::*member)()) {
    return [member](T* p) {
        return std::invoke(member, *p);
    };
}

template <typename R, typename T>
constexpr auto PointerProject(R (T::*member)() const) {
    return [member](const T* p) {
        return std::invoke(member, *p);
    };
}

struct bsearch_includes_fn {
    template <
        std::input_iterator I1, std::sentinel_for<I1> S1, std::input_iterator I2, std::sentinel_for<I2> S2,
        class Proj1 = std::identity, class Proj2 = std::identity,
        std::indirect_strict_weak_order<std::projected<I1, Proj1>, std::projected<I2, Proj2>> Comp = std::ranges::less>
    constexpr bool operator()(I1 first1, S1 last1, I2 first2, S2 last2, Comp comp = {}, Proj1 proj1 = {},
                              Proj2 proj2 = {}) const {
        for (; first2 != last2; ++first2) {
            first1 =
                std::ranges::lower_bound(first1, last1, std::invoke(proj2, *first2), std::ref(comp), std::ref(proj1));
            if (first1 == last1) return false;
            if (std::invoke(comp, std::invoke(proj2, *first2), std::invoke(proj1, *first1))) return false;
        }
        return true;
    }

    template <std::ranges::input_range R1, std::ranges::input_range R2, class Proj1 = std::identity,
              class Proj2 = std::identity,
              std::indirect_strict_weak_order<std::projected<std::ranges::iterator_t<R1>, Proj1>,
                                              std::projected<std::ranges::iterator_t<R2>, Proj2>>
                  Comp = ranges::less>
    constexpr bool operator()(R1&& r1, R2&& r2, Comp comp = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const {
        return (*this)(std::ranges::begin(r1), std::ranges::end(r1), std::ranges::begin(r2), std::ranges::end(r2),
                       std::ref(comp), std::ref(proj1), std::ref(proj2));
    }
};

inline constexpr auto bsearch_includes = bsearch_includes_fn{};

///////////////////////////////////////////////////////////////////////////////////////////////////

void AocMain(std::string_view input);
