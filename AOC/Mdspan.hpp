#pragma once

#include "Attr.hpp"

#include <mdspan/mdarray.hpp>
#include <mdspan/mdspan.hpp>

#include <algorithm>
#include <array>
#include <functional>
#include <ranges>
#include <utility>

using namespace Kokkos;
using Kokkos::Experimental::layout_left_padded;
using Kokkos::Experimental::layout_right_padded;
using Kokkos::Experimental::mdarray;

struct ZYX {
    template <class Self>
        requires(std::tuple_size_v<std::remove_cvref_t<Self>> >= 1)
    [[attr_forceinline]] decltype(auto) x(this Self&& self) {
        return std::forward<Self>(self)[std::tuple_size_v<std::remove_cvref_t<Self>> - 1];
    }

    template <class Self>
        requires(std::tuple_size_v<std::remove_cvref_t<Self>> >= 2)
    [[attr_forceinline]] decltype(auto) y(this Self&& self) {
        return std::forward<Self>(self)[std::tuple_size_v<std::remove_cvref_t<Self>> - 2];
    }

    template <class Self>
        requires(std::tuple_size_v<std::remove_cvref_t<Self>> >= 3)
    [[attr_forceinline]] decltype(auto) z(this Self&& self) {
        return std::forward<Self>(self)[std::tuple_size_v<std::remove_cvref_t<Self>> - 3];
    }
};

template <class IndexType, size_t Rank>
struct Vec : std::array<IndexType, Rank>, ZYX {
    using Base = std::array<IndexType, Rank>;
};

template <class IndexType, size_t Rank>
struct Pos : std::array<IndexType, Rank>, ZYX {
    using Base = std::array<IndexType, Rank>;

    // template <size_t... Extents>
    // Pos(const extents<IndexType, Extents...>& extents) noexcept : Base{} {
    // }

    [[attr_forceinline]]  constexpr Pos& operator+=(IndexType scalar) noexcept {
        for (IndexType& posE : *this) {
            posE += scalar;
        }
        return *this;
    }
    [[attr_forceinline]] constexpr Pos& operator-=(IndexType scalar) noexcept {
        for (IndexType& posE : *this) {
            posE -= scalar;
        }
        return *this;
    }

    [[attr_forceinline]] constexpr Pos& operator+=(const Vec<IndexType, Rank>& vec) noexcept {
        for (size_t r{0}; r < Rank; ++r) {
            (*this)[r] += vec[r];
        }
        return *this;
    }
    [[attr_forceinline]] constexpr Pos& operator-=(const Vec<IndexType, Rank>& vec) noexcept {
        for (size_t r{0}; r < Rank; ++r) {
            (*this)[r] -= vec[r];
        }
        return *this;
    }
};

template <class IndexType, size_t Rank>
[[nodiscard, attr_forceinline]] constexpr Pos<IndexType, Rank> operator+(const Pos<IndexType, Rank>& lhs,
                                                       const Vec<IndexType, Rank>& rhs) noexcept {
    Pos<IndexType, Rank> result{lhs};
    result += rhs;
    return result;
}

template <class IndexType, size_t Rank>
[[nodiscard, attr_forceinline]] constexpr Vec<IndexType, Rank> operator-(const Pos<IndexType, Rank>& lhs,
                                                        const Pos<IndexType, Rank>& rhs) noexcept {
    Vec<IndexType, Rank> result;
    for (size_t r{0}; r < Rank; ++r) {
        result[r] = lhs[r] - rhs[r];
    }
    return result;
}

template <class IndexType, size_t Rank>
[[nodiscard, attr_forceinline]] constexpr bool operator<(const Pos<IndexType, Rank>& lhs, const Pos<IndexType, Rank>& rhs) noexcept {
    return std::ranges::fold_left(std::views::zip_transform(std::less{}, lhs, rhs), true, std::bit_and{});
}

template <class IndexType, std::size_t Rank>
struct std::hash<Vec<IndexType, Rank>> {
    [[nodiscard]] constexpr std::size_t operator()(const Vec<IndexType, Rank>& vec) const noexcept {
        size_t seed = 0;
        for (const IndexType vecE : vec) {
            seed = HashCombine(seed, vecE);
        }
        return seed;
    }
};

template <class IndexType, std::size_t Rank>
struct std::hash<Pos<IndexType, Rank>> {
    [[nodiscard]] constexpr std::size_t operator()(const Pos<IndexType, Rank>& pos) const noexcept {
        size_t seed = 0;
        for (const IndexType posE : pos) {
            seed = HashCombine(seed, posE);
        }
        return seed;
    }
};

template <class IndexType, std::size_t Rank>
struct std::less<Pos<IndexType, Rank>> {
    [[nodiscard]] constexpr bool operator()(const Pos<IndexType, Rank>& lhs,
                                            const Pos<IndexType, Rank>& rhs) const noexcept {
        return std::ranges::lexicographical_compare(lhs, rhs);
    }
};

template <class IndexType, std::size_t Rank>
struct std::equal_to<Pos<IndexType, Rank>> {
    [[nodiscard]] constexpr bool operator()(const Pos<IndexType, Rank>& lhs,
                                            const Pos<IndexType, Rank>& rhs) const noexcept {
        return std::ranges::equal(lhs, rhs);
    }
};

using Vec2D = Vec<int32_t, 2>;
using Pos2D = Pos<int32_t, 2>;

template <class T = const char>
using InputGrid = mdspan<T, dextents<int32_t, 2>, Experimental::layout_right_padded<dynamic_extent>>;

template <std::ranges::contiguous_range Input>
constexpr auto ToGrid(Input& input) {
    size_t width  = std::string_view{input}.find('\n');
    size_t stride = width + 1;
    size_t height = input.size() / stride;
    return mdspan(input.data(), typename InputGrid<std::ranges::range_value_t<Input>>::mapping_type(
                                    dextents<int32_t, 2>(height, width), stride));
}

template <class IndexType, std::size_t Rank>
struct std::tuple_size<Pos<IndexType, Rank>> : public integral_constant<std::size_t, Rank> {};

template <std::size_t I, class IndexType, std::size_t Rank>
struct std::tuple_element<I, Pos<IndexType, Rank>> {
    using type = IndexType;
};

template <std::size_t I, typename IndexType, std::size_t Rank>
    requires(I < Rank)
inline const IndexType& get(const Pos<IndexType, Rank>& arr) {
    return arr[I];
};

template <std::size_t I, typename IndexType, std::size_t Rank>
    requires(I < Rank)
inline IndexType& get(Pos<IndexType, Rank>& arr) {
    return arr[I];
};

template <class IndexType, std::size_t Rank>
struct std::tuple_size<Vec<IndexType, Rank>> : public integral_constant<std::size_t, Rank> {};

template <std::size_t I, class IndexType, std::size_t Rank>
struct std::tuple_element<I, Vec<IndexType, Rank>> {
    using type = IndexType;
};

template <std::size_t I, typename IndexType, std::size_t Rank>
    requires(I < Rank)
[[attr_forceinline]] inline const IndexType& get(const Vec<IndexType, Rank>& arr) {
    return arr[I];
};

template <std::size_t I, typename IndexType, std::size_t Rank>
    requires(I < Rank)
[[attr_forceinline]] inline IndexType& get(Vec<IndexType, Rank>& arr) {
    return arr[I];
};

template <class IndexType, size_t... Extents>
constexpr bool InBounds(const extents<IndexType, Extents...>& extents, const Pos<IndexType, sizeof...(Extents)>& pos) {
    for (size_t r{0}; r < sizeof...(Extents); ++r) {
        if (pos[r] < 0 || pos[r] >= extents.extent(r)) {
            return false;
        }
    }
    return true;
}