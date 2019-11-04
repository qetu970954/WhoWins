cmake_minimum_required(VERSION 3.15)
project(WinrateApproximator)
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(gtest)

add_executable(WinrateApproximator main.cpp)
target_link_libraries(WinrateApproximator gtest)


