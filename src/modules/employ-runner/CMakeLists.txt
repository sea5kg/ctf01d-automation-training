cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

set(PROJECT_NAME employ-runner)

# define project for C language
project(${PROJECT_NAME} C)

message("Building on: ${CMAKE_SYSTEM_NAME}")

set(MODULE_STATIC_NAME "${PROJECT_NAME}-static")

find_package(Threads REQUIRED)

# add include path for project
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/../../../src.wsjcpp/wsjcpp_employees
)

set(SRC_LIB_FILE ${PROJECT_SOURCE_DIR}/src/employ_runner.cpp)
file(GLOB HDR_FILES ${PROJECT_SOURCE_DIR}/src/*.h)

add_library(${MODULE_STATIC_NAME} STATIC ${SRC_LIB_FILE})

