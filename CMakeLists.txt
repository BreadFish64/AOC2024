cmake_minimum_required(VERSION 3.26)

project("AOC" CXX)

include(FetchContent)

set (CMAKE_CXX_STANDARD 23)

set(MDSPAN_PATCH git apply ${CMAKE_CURRENT_SOURCE_DIR}/mdspan_patch.txt)

FetchContent_Declare(
  mdspan
  GIT_REPOSITORY https://github.com/kokkos/mdspan.git
  GIT_TAG cb3bd4284dc3bcdfd8b7779a62ada1504fefd6a4
  # PATCH_COMMAND ${MDSPAN_PATCH}
  UPDATE_DISCONNECTED 1
)
FetchContent_MakeAvailable(mdspan)

add_subdirectory("AOC")
