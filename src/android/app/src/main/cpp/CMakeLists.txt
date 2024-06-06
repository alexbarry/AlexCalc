cmake_minimum_required(VERSION 3.12.2)

set(SRC_DIR "../../../../../")

add_subdirectory("${SRC_DIR}/calc_core" derp_out/calc_core)

list(APPEND COMPILE_OPTS
	"-Wall"
	"-Werror"
	# "-Wno-error=cpp"
	# "-Wno-error=sign-compare"
)

# TODO this must not be working, because I have a ton of
# warnings that would cause errors
#set(CMAKE_C_FLAGS   "${COMPILE_OPTS}")
#set(CMAKE_CXX_FLAGS "${COMPILE_OPTS}")

add_library(calc_android_jni SHARED
            calc_android_jni.cpp)

target_link_libraries(calc_android_jni
                      calc_json
                      calc_core
                      android
                      log)

