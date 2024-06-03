cmake_minimum_required(VERSION 3.4.1)

add_library(calc_core_ui STATIC
            calc_core_ui.cpp)

target_include_directories(calc_core_ui PUBLIC "../calc_core/includes/")
