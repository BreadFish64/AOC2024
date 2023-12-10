find_package(fmt           CONFIG REQUIRED)
find_package(range-v3      CONFIG REQUIRED)
find_package(Boost         REQUIRED)
find_package(Eigen3 3.3    REQUIRED NO_MODULE)
find_package(Microsoft.GSL CONFIG REQUIRED)
find_package(mio           CONFIG REQUIRED)
find_package(lodepng       CONFIG REQUIRED)

add_library(common_pch
    tsc.cpp
    pch.cpp
)
target_link_libraries(common_pch PUBLIC fmt::fmt range-v3 Boost::boost std::mdspan Eigen3::Eigen Microsoft.GSL::GSL mio::mio lodepng)
target_compile_options(common_pch PUBLIC /utf-8 /W4)
target_compile_definitions(common_pch PUBLIC MDSPAN_USE_BRACKET_OPERATOR=0 _MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS=0)
target_precompile_headers(common_pch PUBLIC pch.hpp)

function(Problem problem_name)

add_executable(${problem_name}
    ${problem_name}.cpp
)

target_compile_options(${problem_name} PRIVATE /utf-8 /W4)
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

Problem(chronal_calibration)
Problem(trebuchet)
Problem(cube_conundrum)
Problem(gear_ratios)
Problem(scratchcards)
Problem(if_you_give_a_seed_a_fertilizer)
Problem(wait_for_it)
Problem(camel_cards)
Problem(haunted_wasteland)
Problem(mirage_maintenance)
Problem(pipe_maze)