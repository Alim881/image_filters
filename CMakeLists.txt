cmake_minimum_required(VERSION 3.10)
project(ImageFilters)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(PNG_TESTS OFF CACHE BOOL "" FORCE)
set(PNG_SHARED OFF CACHE BOOL "" FORCE)
set(PNG_EXECUTABLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG v1.3.1
)

FetchContent_Declare(
    libpng
    GIT_REPOSITORY https://github.com/glennrp/libpng.git
    GIT_TAG v1.6.43
)

FetchContent_MakeAvailable(zlib libpng)

add_library(image_filters_lib STATIC src/image_filters.cpp)

target_include_directories(image_filters_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE
        ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR}
        ${libpng_SOURCE_DIR} ${libpng_BINARY_DIR}
)

target_link_libraries(image_filters_lib
    PRIVATE png_static zlibstatic
)

add_executable(image_filters main.cpp)
target_link_libraries(image_filters PRIVATE image_filters_lib)
target_include_directories(image_filters
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${libpng_SOURCE_DIR}
        ${libpng_BINARY_DIR}
)

add_executable(tests test/test_image_filters.cpp)
target_link_libraries(tests PRIVATE image_filters_lib)

target_include_directories(tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/external  
        ${libpng_SOURCE_DIR}
        ${libpng_BINARY_DIR}
)

enable_testing()
add_test(NAME AllTests COMMAND tests)
