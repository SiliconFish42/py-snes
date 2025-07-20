# Minimum CMake version and project definition
cmake_minimum_required(VERSION 3.15)
project(pysnes CXX)

# Set C++ standard to C++17 for filesystem support
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the necessary packages for the main library
find_package(pybind11 REQUIRED)
find_package(Python COMPONENTS Interpreter Development REQUIRED)

# Define our Python extension module (commented out for now)
pybind11_add_module(pysnes_cpp
    src/pysnes/bindings.cpp
    src/pysnes/snes/src/snes.cpp
    src/pysnes/snes/src/cpu.cpp
    src/pysnes/snes/src/ppu.cpp
    src/pysnes/snes/src/bus.cpp
    src/pysnes/snes/src/cartridge.cpp
    src/pysnes/snes/src/controller.cpp
)

# Add our local include directory
target_include_directories(pysnes_cpp PRIVATE src/pysnes/snes/include)

# Install the final compiled library into the 'pysnes' package directory.
install(TARGETS pysnes_cpp DESTINATION pysnes)


# --------------------- TESTING SETUP (FetchContent Method) --------------------

# Enable testing
enable_testing()

# Include the FetchContent module
include(FetchContent)

# Declare the Google Test dependency. CMake will download it from GitHub.
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG    v1.17.0
)

# Make the Google Test project available to our build
FetchContent_MakeAvailable(googletest)

# Create our test executable (only in build directory)
add_executable(
    run_tests
    tests/test_cpu.cpp
    tests/test_control.cpp
    tests/test_arithmetic_v2.cpp
    tests/test_incdec.cpp
    tests/test_comparison.cpp
    tests/test_logical.cpp
    tests/test_shift_rotate.cpp
    tests/test_transfer.cpp
    tests/test_stack.cpp
    tests/test_branch.cpp
    tests/test_jump_subroutine.cpp
    tests/test_status_flags.cpp
    tests/test_rom.cpp
    tests/test_framework.cpp
    tests/test_framework_tests.cpp
    # tests/test_ppu.cpp
    src/pysnes/snes/src/cpu.cpp
    src/pysnes/snes/src/cpu_addressing.cpp
    src/pysnes/snes/src/cpu_helpers.cpp
    src/pysnes/snes/src/cpu_instructions.cpp
    src/pysnes/snes/src/bus.cpp
    src/pysnes/snes/src/ppu.cpp
    src/pysnes/snes/src/controller.cpp
    src/pysnes/snes/src/cartridge.cpp
)

# Ensure the executable is built in the build directory only
set_target_properties(run_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Add our project's include directory to the test build
target_include_directories(run_tests PRIVATE src/pysnes/snes/include)

# Link our test executable against the Google Test libraries
target_link_libraries(
    run_tests PRIVATE
    GTest::gtest_main # This target is created by FetchContent
)

add_test(NAME pysnes_tests COMMAND $<TARGET_FILE:run_tests>)
# Set profiling output directory for coverage tests
if(CODE_COVERAGE)
  set_tests_properties(pysnes_tests PROPERTIES
    ENVIRONMENT "LLVM_PROFILE_FILE=${CMAKE_BINARY_DIR}/default.profraw"
  )
endif()

add_custom_target(
    check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_dependencies(check run_tests)
# add_dependencies(pysnes_cpp check)

option(CODE_COVERAGE "Enable coverage reporting" ON)
if((CODE_COVERAGE) AND (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
  # Set profiling output directory to build directory to prevent .profraw files in root
  set_target_properties(run_tests PROPERTIES
    COMPILE_FLAGS "-fprofile-instr-generate -fcoverage-mapping"
    LINK_FLAGS "-fprofile-instr-generate -fcoverage-mapping"
  )
endif()
