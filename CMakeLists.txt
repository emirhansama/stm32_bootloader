# Set the minimum version of CMake that can be used
cmake_minimum_required(VERSION 3.20)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

# Read MCU selection from bl_config.h
set(BL_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/user/app/bl_config.h")

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${BL_CONFIG_FILE}")

file(STRINGS "${BL_CONFIG_FILE}" BL_MCU_LINE REGEX "^[ \t]*#define[ \t]+BL_MCU[ \t]+BL_MCU_STM32F(407|746)")

if(BL_MCU_LINE MATCHES "BL_MCU_STM32F407")
    set(BL_TARGET_MCU "stm32f407")
elseif(BL_MCU_LINE MATCHES "BL_MCU_STM32F746")
    set(BL_TARGET_MCU "stm32f746")
else()
    message(FATAL_ERROR "Invalid MCU selection in bl_config.h")
endif()

message(STATUS "Bootloader target MCU: ${BL_TARGET_MCU}")

# Set target definition variables required by the toolchain file
include("${CMAKE_CURRENT_SOURCE_DIR}/platform/${BL_TARGET_MCU}/target.cmake")

# Set the toolchain file for cross-compilation
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/gcc-arm-none-eabi.cmake)

# Project definition
project(bl)

# Enable CMake support for ASM and C/CPP languages
enable_language(C ASM)

set(PROJECT_NAME bl)

# Create an executable object type
add_executable(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/platform/${BL_TARGET_MCU}"
)
# Include additional CMake files for flags, components, and files
include("${CMAKE_CURRENT_SOURCE_DIR}/platform/${BL_TARGET_MCU}/flags.cmake")
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/components.cmake)
include("${CMAKE_CURRENT_SOURCE_DIR}/platform/${BL_TARGET_MCU}/files.cmake")

# Add sources to executable/library
target_sources(${PROJECT_NAME} PRIVATE
  # User defined sources
)

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE
  # User defined include directories
)

# Compiler options
target_compile_options(${PROJECT_NAME} PRIVATE
  # User defined compiler options
)

# Add additional defined symbols
target_compile_definitions(${PROJECT_NAME} PRIVATE
  # Add additional defined symbols here
)

# Linker options
target_link_options(${PROJECT_NAME} PRIVATE 
  # User defined linker options
)

# Add additional linker search directories
target_link_directories(${PROJECT_NAME} PRIVATE
  # Add additional linker search directories here
)

# Add additional linker libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
  # Add additional linker libraries here
)
