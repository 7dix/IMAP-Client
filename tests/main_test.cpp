CMakeLists.txt:
cmake_minimum_required(VERSION 3.10)
project(project-name)

enable_testing()

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(runTests tests/main_test.cpp)
target_link_libraries(runTests ${GTEST_LIBRARIES} pthread)

add_test(NAME MyTests COMMAND runTests)

tests/main_test.cpp:
#include <gtest/gtest.h>

TEST(HelloWorldTest, BasicAssertions) {
    EXPECT_EQ(1, 1);
    EXPECT_TRUE(true);
}