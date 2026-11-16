cmake_minimum_required(VERSION 3.16)

# set the project name
project(driver_test LANGUAGES CXX)

option(FATFS_ETL_NO_STL "build with ETL instead of STL" OFF)

if(FATFS_ETL_NO_STL)
  add_compile_options(-DETL_NO_STL -DETL_NO_INITIALIZER_LIST -DETL_NO_EXCEPTIONS)
endif()

set (DCMAKE_CXX_FLAGS -Werror -Wall -Wextra)
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

add_compile_options("-xc++")
SET_SOURCE_FILES_PROPERTIES(driver-test-ram.ino PROPERTIES LANGUAGE CXX)

# build sketch as executable  
add_executable (driver_test driver-test-ram.ino )

# link with fatfs
target_link_libraries(driver_test arduino_fatfs)
