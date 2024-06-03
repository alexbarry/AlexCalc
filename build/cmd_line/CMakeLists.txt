cmake_minimum_required(VERSION 3.4.1)
#include("src/calc_core/CMakeLists.txt")

# So far this works by doing:
#     cd build
#     cmake ../
#     cmake --build .

set(SRC_DIR "../../src")

set(CMAKE_CXX_COMPILER "clang++")

list(APPEND COMPILE_OPTS
	"-Wall"
	"-Werror"
	"-Wno-error=cpp"
	"-Wno-error=sign-compare"
	"-O3"
	"-g"
	"-std=c++11"
)

add_compile_options("${COMPILE_OPTS}")
add_subdirectory("${SRC_DIR}/calc_core" out/calc_core)

add_executable (calc
                "${SRC_DIR}/calc_core/calc_main.cpp")
target_link_libraries(calc PRIVATE calc_core)
