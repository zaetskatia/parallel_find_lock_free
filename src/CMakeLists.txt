cmake_minimum_required(VERSION 3.0)

project(test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories(src)

# Add all source files to a variable
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Specify the target executable and its source files
add_executable(test ${SOURCES})

# Link pthread library
target_link_libraries(test PRIVATE pthread)
