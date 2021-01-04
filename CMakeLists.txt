cmake_minimum_required(VERSION 3.17)
project(servo_control)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(servo_control
        servo.cpp
        servo.h)
