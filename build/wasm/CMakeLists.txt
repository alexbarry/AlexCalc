cmake_minimum_required(VERSION 3.4.1)

# TODO I don't know how to make this work.
# What I want is to be able to call cmake from any directory, but
# point it to this file CMakeLists.txt in build/wasm
# So `cmake build/wasm`
# But then it puts the outputs from wherever it is run, such as  my git repo root
# For now, as a workaround, I am calling cmake build/wasm in a shell script after 
# `cd`ing to the location of the build script (from within the script).
# That seems messy. I will probably want to be able to pass arguments to
# cmake
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "lib")
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "lib")
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "out")

set(CMAKE_CXX_COMPILER "emcc")

list(APPEND EXPORTED_FUNCTIONS_WASM
     "[ \
        \"_alexcalc_info_func\", \
        \"_alexcalc_init\", \
        \"_alexcalc_json_str_output\", \"_alexcalc_to_latex\", \
         \"_alexcalc_new_calcdata\", \"_alexcalc_free_calcdata\", \
         \"_alexcalc_calcdata_to_json\", \
         \"_alexcalc_data_state_set\", \
         \"_alexcalc_get_unit_info_json\", \
         \"_alexcalc_add_recently_used_unit\", \
         \"_alexcalc_get_recently_used_units_json\" \
        ]")


list(APPEND EMCC_LINK_OPTS "-Wall")
# TODO enable when the linker warning is fixed 
# emcc: error: out/calc_core/libcalc_json.a: archive is missing an index; Use emar when creating libraries to ensure an index is created [-Wemcc] [-Werror]
# list(APPEND EMCC_LINK_OPTS "-Werror")
list(APPEND EMCC_LINK_OPTS "-s DISABLE_EXCEPTION_CATCHING=0")
list(APPEND EMCC_LINK_OPTS "-s EXPORTED_FUNCTIONS='${EXPORTED_FUNCTIONS_WASM}'")
list(APPEND EMCC_LINK_OPTS "-s EXPORTED_RUNTIME_METHODS='[\"ccall\", \"cwrap\"]'")
list(APPEND EMCC_LINK_OPTS "-s NO_EXIT_RUNTIME=1")
# list(APPEND EMCC_LINK_OPTS "-s ERROR_ON_UNDEFINED_SYMBOLS=1")

list(APPEND EMCC_COMPILE_OPTS
	"-s" "DISABLE_EXCEPTION_CATCHING=0"
	"-O3"
	"-Wall"
	"-Werror"
	"-Wno-error=cpp"
	# TODO this doesn't seem to be needed, I guess emscripten doesn't
	# check for the sign comparisons, even with -Werror set?
	# "-Wno-error=sign-compare"
)

set(SRC_DIR "../../src")

add_compile_options("${EMCC_COMPILE_OPTS}")

add_subdirectory ("${SRC_DIR}/calc_core" out/calc_core)
set(CMAKE_SHARED_LINKER_FLAGS "${EMCC_LINK_OPTS}")

# On linux, I get an error if I don't provide any sources when calling `add_executable`
# So add this "empty.cpp" file.
add_executable (calc_wasm.js "${SRC_DIR}/calc_core/empty.cpp")
                
target_compile_options(calc_wasm.js PRIVATE "${EMCC_COMPILE_OPTS}")

target_compile_options(calc_core PRIVATE "${EMCC_COMPILE_OPTS}")
target_link_libraries(calc_wasm.js PRIVATE calc_json "${EMCC_LINK_OPTS}")
