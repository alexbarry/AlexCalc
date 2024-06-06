cmake_minimum_required(VERSION 3.12.2)

#add_library(hello-jni SHARED
#            hello-jni.c)

# Include libraries needed for hello-jni lib
#target_link_libraries(hello-jni
#                      android
#                      log)
#

#include_directories("includes")

add_subdirectory("../calc_core_ui" out/calc_core_ui/)

add_library(calc_core STATIC
            calc_core.cpp
            calc_core_exceptions.cpp
            calc_parse.cpp
            calc_units.cpp
            calc_units_organizer.cpp
            node_to_latex.cpp
            misc.cpp)
target_include_directories(calc_core PUBLIC "includes")
target_include_directories(calc_core PUBLIC "../calc_core_ui/")

string(TIMESTAMP BUILD_DATETIME "%Y-%m-%d %H:%M:%S%z")
# TODO add a variable here that can be set to indicate who built it,
# e.g. github pages
add_compile_definitions(ALEXCALC_BUILD_INFO="Built on ${BUILD_DATETIME}")

add_library(calc_json STATIC
            calc_json.cpp)
target_link_libraries(calc_json calc_core calc_core_ui)
