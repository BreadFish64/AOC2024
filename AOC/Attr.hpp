#pragma once

#if defined(__GNUC__) || defined(__clang__)
#define attr_noinline gnu::noinline
#define attr_forceinline gnu::always_inline
#define attr_flatten gnu::flatten
#else
#define attr_noinline msvc::noinline
#define attr_forceinline msvc::forceinline
#define attr_flatten
#endif