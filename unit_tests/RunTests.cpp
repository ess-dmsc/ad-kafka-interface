/** Copyright (C) 2017 European Spallation Source */

/** @file  main.cpp
 *  @brief The starting point of the unit tests.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

/// @brief Initializes gtest and gmock and runs the unit tests.
int main(int argc, char **argv) {
  // Perform death tests in a separate process
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
