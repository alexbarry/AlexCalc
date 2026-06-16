cmake_minimum_required(VERSION 3.12.2)

set(SRC_DIR "../../../../../")

set(CMAKE_CXX_STANDARD 20)

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

include(FetchContent)

FetchContent_Declare(
	calc_android_jni_experimental
	GIT_REPOSITORY https://github.com/alexbarry/AlexCalc.git
	GIT_TAG main-experimental
)

FetchContent_GetProperties(calc_android_jni_experimental)

if (true)


	FetchContent_Populate(calc_android_jni_experimental)

	SET(exp_src "${calc_android_jni_experimental_SOURCE_DIR}/src/calc_core")
	SET(exp_bin "${calc_android_jni_experimental_BINARY_DIR}")
	message("calling add_subdirectory with exp_src=${exp_src}; exp_bin=${exp_bin}")

	set(BUILD_LIB_SUFFIX "-experimental")

	# All the CMake libraries defined in this subdirectory should
	# have -experimental appended to them.
	add_subdirectory(${exp_src}
	                 ${exp_bin})

	# Re-build this file, but it will reference
	# the headers defined in the experimental branch.
	# (And be linked against the experimental binary)
	add_library(calc_android_jni-experimental SHARED
			calc_android_jni.cpp)

	target_link_libraries(calc_android_jni-experimental
	                      calc_json-experimental
	                      android
	                      log)

endif()
