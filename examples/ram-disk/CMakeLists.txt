cmake_minimum_required(VERSION 3.16)

# set the project name
project(ram_disk LANGUAGES CXX)

option(FATFS_ETL_NO_STL "build with ETL instead of STL" OFF)

if(FATFS_ETL_NO_STL)
  add_compile_options(-DETL_NO_STL -DETL_NO_INITIALIZER_LIST -DETL_NO_EXCEPTIONS)
endif()

set (DCMAKE_CXX_FLAGS -Werror -Wall -Wextra)

add_compile_options(-xc++ -fsanitize=address)
add_link_options(-fsanitize=address)

SET_SOURCE_FILES_PROPERTIES(ram-disk.ino PROPERTIES LANGUAGE CXX)

# build sketch as executable  
add_executable (ram_disk ram-disk.ino )

# link with fatfs
target_link_libraries(ram_disk PUBLIC arduino_fatfs)
