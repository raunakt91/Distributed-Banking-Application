cmake_minimum_required(VERSION 3.8)
project(Distributed_Banking_Application)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES client.c)
add_executable(Distributed_Banking_Application ${SOURCE_FILES})