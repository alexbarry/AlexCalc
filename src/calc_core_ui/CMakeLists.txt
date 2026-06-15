
add_library(calc_core_ui${BUILD_LIB_SUFFIX} STATIC
            calc_core_ui.cpp)

target_include_directories(calc_core_ui${BUILD_LIB_SUFFIX} PUBLIC "../calc_core/includes/")
