cmake_minimum_required(VERSION 3.16)

# set the project name
project(arduino_fatfs)

# lots of warnings and all warnings as errors
## add_compile_options(-Wall -Wextra )
set(CMAKE_CXX_STANDARD 17)
set (DCMAKE_CXX_FLAGS -Werror -Wall -Wextra)

option(FATFS_EXAMPES "build examples" ON)
option(FATFS_BUILD_TESTS "build the desktop ctest suite" ON)
option(FATFS_SANITIZE "build with -fsanitize=address" ON)
option(FATFS_ETL_NO_STL "build with ETL instead of STL" OFF)

# define libraries
add_library (arduino_fatfs INTERFACE)

# define location for header files
target_include_directories(arduino_fatfs INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src  )
if(FATFS_SANITIZE)
  add_compile_options(-fsanitize=address)
  add_link_options(-fsanitize=address)
endif()

# build examples
if(FATFS_EXAMPES)
  add_subdirectory( "${CMAKE_CURRENT_SOURCE_DIR}/examples/ram-disk")
  add_subdirectory( "${CMAKE_CURRENT_SOURCE_DIR}/examples/driver-test-ram")
endif()

# desktop ctest suite (hardware-free drivers only: RamIO, StreamIO, MultiIO)
if(FATFS_BUILD_TESTS)
  enable_testing()
  add_subdirectory( "${CMAKE_CURRENT_SOURCE_DIR}/tests")
endif()

# ETL is Embedded Template Library. No dynamic allocations. For embedded devices.
if(FATFS_ETL_NO_STL)
  add_compile_options(-DETL_NO_STL -DETL_NO_INITIALIZER_LIST -DETL_NO_EXCEPTIONS)

  find_path(ETL_INCLUDE_DIR NAMES Embedded_Template_Library.h etl/string.h etl/vector.h
    HINTS ${CMAKE_SOURCE_DIR}/../etl ${CMAKE_SOURCE_DIR}/../../etl ${CMAKE_SOURCE_DIR}/../Embedded_Template_Library_ETL/src
    DOC "Path to ETL (Embedded Template Library) headers")
  if(ETL_INCLUDE_DIR)
    target_include_directories(arduino_fatfs INTERFACE ${ETL_INCLUDE_DIR})
  else()
    message(WARNING "ETL not found. Set ETL_INCLUDE_DIR to the ETL root directory.")
  endif()
endif()
