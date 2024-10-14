cmake_minimum_required(VERSION 3.17)

project(tests)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(../modules/gtest build)

add_executable(tests
    ../engine/Easing.cpp

    main.cpp
    AStarSearchTest.cpp
    SignalTest.cpp
    TweenTest.cpp
)

target_link_libraries(tests gtest_main gmock_main)

add_test(NAME tests COMMAND tests)