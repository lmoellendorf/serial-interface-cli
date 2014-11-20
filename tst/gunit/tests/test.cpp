/*
 * test.cpp
 *
 *  Created on: 13.11.2014
 *      Author: Naksit Anantalapochai
 */

#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test.h"

int main(int argc, char **argv)
{
  /* Initializes Google Mock framework. */
  ::testing::InitGoogleMock(&argc, argv);

  /* Filter to run only the selected test suites. */
  ::testing::GTEST_FLAG(filter) = std::string(UNIT_TEST_SELECTED_SUITE);
  /* Initializes Google Test framework. */
  ::testing::InitGoogleTest(&argc, argv);

  /* Runs the test. */
  return RUN_ALL_TESTS();
} /* main() */
