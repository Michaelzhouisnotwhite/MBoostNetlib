cmake_minimum_required(VERSION 3.22)
include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/3rd_party.cmake)
set(include_dir ${PROJECT_SOURCE_DIR})
message("---------------include dir: ${include_dir}")
add_subdirectory(src)
add_subdirectory(test)