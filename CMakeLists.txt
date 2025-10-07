cmake_minimum_required(VERSION 3.29)

project("AOC" CXX)

include(FetchContent)

set(BOOST_INCLUDE_LIBRARIES container icl pool unordered)
set(BOOST_ENABLE_CMAKE ON)
FetchContent_Declare(
  Boost
  EXCLUDE_FROM_ALL YES
  URL https://github.com/boostorg/boost/releases/download/boost-1.89.0/boost-1.89.0-cmake.7z
  URL_HASH SHA256=252149f583054515b10688855b24ca1c16072b5118e603b8fe017894d319c262
)

FetchContent_Declare(
  mdspan
  EXCLUDE_FROM_ALL YES
  GIT_REPOSITORY https://github.com/kokkos/mdspan.git
  GIT_TAG 3c1aa524617f272836a7222e69d17574ce998dd8
)

FetchContent_Declare(
  range-v3
  EXCLUDE_FROM_ALL YES
  GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
  GIT_TAG ca1388fb9da8e69314dda222dc7b139ca84e092f
)

FetchContent_Declare(
  eigen
  EXCLUDE_FROM_ALL YES
  URL https://gitlab.com/libeigen/eigen/-/archive/5.0.0/eigen-5.0.0.tar.gz
  URL_HASH SHA256=315c881e19e17542a7d428c5aa37d113c89b9500d350c433797b730cd449c056
)

FetchContent_MakeAvailable(Boost mdspan range-v3 eigen)

add_subdirectory("AOC")
