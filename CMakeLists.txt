cmake_minimum_required(VERSION 3.0)

project(CMR)

find_package(Threads REQUIRED)

include_directories(.)
include_directories(boost_1_64_0/)

add_subdirectory(exprCalc)
