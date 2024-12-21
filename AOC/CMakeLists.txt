find_package(range-v3      CONFIG REQUIRED)
find_package(Boost         REQUIRED)
find_package(Eigen3 3.3    REQUIRED NO_MODULE)

if (MSVC)
    set(EXTRA_FLAGS /utf-8 /W4 /WX /FA)
else()
    set(EXTRA_FLAGS -Wall -Wextra -Werror)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND EXTRA_FLAGS -Wno-missing-braces)
endif()

add_executable(MdspanTest
    MdspanTest.cpp
)

target_link_libraries(MdspanTest PRIVATE std::mdspan)
target_compile_options(MdspanTest PRIVATE ${EXTRA_FLAGS})

add_library(common_pch
    pch.cpp
    TscClock.cpp
)
target_include_directories(common_pch PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/override)
target_link_libraries(common_pch PUBLIC range-v3 Boost::boost std::mdspan Eigen3::Eigen)
target_compile_options(common_pch PUBLIC ${EXTRA_FLAGS})
target_compile_definitions(common_pch PUBLIC MDSPAN_USE_BRACKET_OPERATOR=0)
if (MSVC)
    target_compile_definitions(common_pch PUBLIC WIN32_LEAN_AND_MEAN NOMINMAX)
endif()
target_precompile_headers(common_pch PUBLIC pch.hpp)

function(Problem problem_name)

add_executable(${problem_name}
    ${problem_name}.cpp
)

target_compile_options(${problem_name} PRIVATE ${EXTRA_FLAGS})
target_link_libraries(${problem_name} PRIVATE common_pch)
target_precompile_headers(${problem_name} REUSE_FROM common_pch)

set_target_properties(${problem_name}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${problem_name}"
)

add_custom_command(
        TARGET ${problem_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/input/${problem_name}.txt
                ${CMAKE_BINARY_DIR}/${problem_name}/input.txt)

endfunction()

Problem(historian_hysteria)
Problem(red_nosed_reports)
Problem(mull_it_over)
Problem(ceres_search)
Problem(print_queue)
Problem(guard_gallivant)
Problem(bridge_repair)
Problem(resonant_collinearity)
Problem(disk_fragmenter)
Problem(hoof_it)
Problem(plutonian_pebbles)
Problem(garden_groups)
Problem(claw_contraption)
Problem(restroom_redoubt)
Problem(warehouse_woes)
Problem(reindeer_maze)
Problem(chronospatial_computer)
Problem(ram_run)
Problem(linen_layout)
Problem(race_condition)
Problem(keypad_conundrum)
