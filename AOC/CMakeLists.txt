add_executable(MdspanTest
    MdspanTest.cpp
)

target_link_libraries(MdspanTest PRIVATE mdspan::mdspan)

set(DO_PRECOMPILE TRUE)

add_library(common_pch
    pch.cpp
    TscClock.cpp
)
target_include_directories(common_pch PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/override)
target_link_libraries(common_pch PUBLIC
    Boost::container
    Boost::icl
    Boost::pool
    Boost::unordered

    mdspan::mdspan
    range-v3
    Eigen3::Eigen
    )
if (WIN32)
    target_compile_definitions(common_pch PUBLIC WIN32_LEAN_AND_MEAN NOMINMAX)
endif()
if (${DO_PRECOMPILE})
    target_precompile_headers(common_pch PUBLIC pch.hpp)
else()
    target_compile_options(common_pch PUBLIC --include=${CMAKE_CURRENT_SOURCE_DIR}/pch.hpp)
endif()

function(Problem problem_name)

add_executable(${problem_name}
    ${problem_name}.cpp
)

target_link_libraries(${problem_name} PRIVATE common_pch)
if (${DO_PRECOMPILE})
  target_precompile_headers(${problem_name} REUSE_FROM common_pch)
endif()

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
Problem(monkey_market)
Problem(lan_party)
Problem(crossed_wires)
Problem(code_chronicle)