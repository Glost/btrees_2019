cmake_minimum_required(VERSION 3.12)
project(btrees CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -static-libgcc -static-libstdc++")

add_subdirectory(root/prj/0.1/sol/projects/btrees_c/src)
add_subdirectory(root/prj/0.1/sol/projects/btrees_coef_exps/src)
