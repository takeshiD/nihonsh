cmake_minimum_required(VERSION 3.14)
project(nihonsh
    VERSION 0.1
    LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 14)
set(TARGET nish)
set(SRCS
    src/prompt.cpp
    src/parse.cpp
    src/complete.cpp
    src/builtin.cpp
    src/jobs.cpp
)
add_executable(${TARGET}
    src/main.cpp
    ${SRCS}
)
target_include_directories(${TARGET} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(${TARGET} PUBLIC
    curses
)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(TEST_NAME test-${PROJECT_NAME})
set(TESTS
    tests/test_parse.cpp
)
add_executable(${TEST_NAME}
    ${SRCS}
    ${TESTS}
)
target_link_libraries(${TEST_NAME}
    gtest_main
    curses
)
target_include_directories(${TEST_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

enable_testing()
add_test(NAME test COMMAND ${TEST_NAME})