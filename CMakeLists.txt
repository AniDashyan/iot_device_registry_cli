cmake_minimum_required(VERSION 3.10)

project(iot_device_registry)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(device_registry main.cpp Device.cpp Device.h)
target_compile_options(device_registry PRIVATE -Wall -Wextra -Werror)