cmake_minimum_required(VERSION 3.16)
project(virtual_six_servo_controller VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(servo_motion src/joint.cpp src/parser.cpp src/controller.cpp src/config.cpp)
target_include_directories(servo_motion PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(one_servo_demo src/one_servo_demo.cpp)
target_link_libraries(one_servo_demo PRIVATE servo_motion)

add_executable(servo_controller src/main.cpp)
target_link_libraries(servo_controller PRIVATE servo_motion)

include(CTest)
if(BUILD_TESTING)
    add_executable(joint_tests tests/test_joint.cpp)
    target_link_libraries(joint_tests PRIVATE servo_motion)
    add_test(NAME joint_motion COMMAND joint_tests)
    add_executable(parser_tests tests/test_parser.cpp)
    target_link_libraries(parser_tests PRIVATE servo_motion)
    add_test(NAME command_parser COMMAND parser_tests)
    add_executable(controller_tests tests/test_controller.cpp)
    target_link_libraries(controller_tests PRIVATE servo_motion)
    add_test(NAME six_joint_controller COMMAND controller_tests)
    find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)
    add_test(NAME python_host_and_integration
        COMMAND "${CMAKE_COMMAND}" -E env
            "PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR}/python"
            "SERVO_CONTROLLER=$<TARGET_FILE:servo_controller>"
            "${Python3_EXECUTABLE}" -m unittest discover
            -s "${CMAKE_CURRENT_SOURCE_DIR}/tests" -p "test_*.py" -v)
    set_tests_properties(python_host_and_integration PROPERTIES TIMEOUT 30)
endif()
