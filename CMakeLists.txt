# Minimum CMake version
cmake_minimum_required(VERSION 3.15...3.27)
# Project name and language
project(
  ${SKBUILD_PROJECT_NAME}
  VERSION ${SKBUILD_PROJECT_VERSION}
  LANGUAGES CXX)

find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)

# Define the Python extension module.
# The name 'pysnes_cpp' must match what you use in your Python code.
pybind11_add_module(pysnes_cpp
    src/pysnes/bindings.cpp
    src/pysnes/snes/src/snes.cpp
    src/pysnes/snes/src/cpu.cpp
    src/pysnes/snes/src/ppu.cpp
    src/pysnes/snes/src/bus.cpp
    src/pysnes/snes/src/cartridge.cpp
    src/pysnes/snes/src/controller.cpp
)

# Add the include directory for your emulator's headers
target_include_directories(pysnes_cpp PRIVATE src/pysnes/snes/include)

set_target_properties(pysnes_cpp PROPERTIES CXX_VISIBILITY_PRESET default)

# This handles installing the final compiled library (.dylib)
# into the correct package directory.
install(TARGETS pysnes_cpp DESTINATION pysnes)