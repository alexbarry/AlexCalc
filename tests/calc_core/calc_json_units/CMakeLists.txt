cmake_minimum_required(VERSION 3.4.1)

set(SRC_DIR "../../../src")

add_subdirectory("${SRC_DIR}/calc_core" out/calc_core)

add_executable(calc_units_json_test
			   calc_json_units.cpp)
target_include_directories(calc_units_json_test PUBLIC "${SRC_DIR}/calc_core/includes")
target_link_libraries(calc_units_json_test PRIVATE calc_core calc_json)

