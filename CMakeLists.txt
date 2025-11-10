cmake_minimum_required(VERSION 3.16)
project(rsaShit)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE RSASHIT_SOURCES CONFIGURE_DEPENDS
        "src/*.cpp"
)

add_executable(rsaShit
        main.cpp
        ${RSASHIT_SOURCES}
)

# find gmp
find_path(GMP_INCLUDE_DIR NAMES gmp.h)
find_library(GMP_LIB NAMES gmp)
if (NOT GMP_INCLUDE_DIR OR NOT GMP_LIB)
    message(FATAL_ERROR "GMP not found; please install (e.g. brew install gmp)")
endif ()

message(STATUS "GMP include: ${GMP_INCLUDE_DIR}")
message(STATUS "GMP lib: ${GMP_LIB}")

target_include_directories(rsaShit PRIVATE ${GMP_INCLUDE_DIR})
target_link_libraries(rsaShit PRIVATE ${GMP_LIB})
