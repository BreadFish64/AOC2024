#pragma once

#include <bit>
#include <cstdint>
#include <span>
#include <type_traits>

struct Fnv1a {
    static constexpr std::uint64_t OFFSET_BASIS = 0xcbf29ce484222325;

    template <std::size_t N>
    static constexpr std::uint64_t Compute(std::span<const std::byte, N> rep,
                                           std::uint64_t hash = OFFSET_BASIS) noexcept {
        for (const std::byte repByte : rep) {
            hash ^= static_cast<std::uint64_t>(repByte);
            hash *= 0x100000001b3;
        }
        return hash;
    }

    template <class T>
        requires(std::is_trivially_destructible_v<T>)
    constexpr std::size_t operator()(const T& v, std::uint64_t hash = OFFSET_BASIS) noexcept {
        if (std::is_constant_evaluated()) {
            return Compute(std::span<const std::byte, sizeof(v)>(std::bit_cast<std::array<std::byte, sizeof(v)>>(v)), hash);
        } else {
            return Compute(std::span<const std::byte, sizeof(v)>(reinterpret_cast<const std::byte*>(&v), sizeof(v)),
                           hash);
        }
    }
};
//
// #include <immintrin.h>
//
// struct CrcHash {
//    template <std::size_t N>
//    static constexpr std::uint32_t Compute(std::span<const std::byte, N> rep, std::uint32_t crc = 0) noexcept {
//        crc = ~crc;
//        std::size_t i{0};
//        while (i + 8 <= rep.size()) {
//            i += 8;
//            std::uint64_t block{};
//            std::memcpy(&block, &rep[i], sizeof(block));
//            crc = _mm_crc32_u64(crc, block);
//        }
//        if (i + 4 <= rep.size()) {
//            i += 4;
//            std::uint32_t block{};
//            std::memcpy(&block, &rep[i], sizeof(block));
//            crc = _mm_crc32_u32(crc, block);
//        }
//        if (i + 2 <= rep.size()) {
//            i += 2;
//            std::uint16_t block{};
//            std::memcpy(&block, &rep[i], sizeof(block));
//            crc = _mm_crc32_u16(crc, block);
//        }
//        if (i + 4 <= rep.size()) {
//            i += 4;
//            std::uint8_t block{};
//            std::memcpy(&block, &rep[i], sizeof(block));
//            crc = _mm_crc32_u8(crc, block);
//        }
//        return ~crc;
//    }
//
//    template <class T>
//        requires(std::has_unique_object_representations_v<T> && std::is_trivially_copyable_v<T> &&
//                 std::is_trivially_destructible_v<T>)
//    constexpr std::size_t operator()(const T& v, std::uint32_t crc = 0) noexcept {
//        if (std::is_constant_evaluated()) {
//            return Compute(std::bit_cast<std::array<std::byte, sizeof(v)>>(v), crc);
//        } else {
//            return Compute(std::span<const std::byte, sizeof(v)>(reinterpret_cast<const std::byte*>(&v), sizeof(v)),
//                           crc);
//        }
//    }
//};