project(async)
cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
include_directories(include)

find_package(Boost REQUIRED container)

add_compile_options(-fsanitize=address) #  -ftime-trace
add_link_options(-fsanitize=address)

add_subdirectory(test)