cmake_minimum_required(VERSION 3.10)
project(snow)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create unified snow interface library, including all third-party libraries
add_library(snow INTERFACE)
add_library(snow::snow ALIAS snow)

# Set include directories (all third-party library header paths + snow's own include directory)
target_include_directories(snow INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/eigen
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# Link thread library (required by httplib)
# On Windows, pthread is not available with MSVC, use Threads::Threads instead
# On Unix-like systems, use pthread
if(WIN32)
    find_package(Threads REQUIRED)
    target_link_libraries(snow INTERFACE Threads::Threads)
else()
    target_link_libraries(snow INTERFACE pthread)
endif()

