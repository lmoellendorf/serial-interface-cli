/*============================================================================*/
/*! \file   test_sample.cpp

    \author

    \brief  Unit test sample code

   \version
==============================================================================*/

/*==============================================================================
                                 INCLUDE FILES
==============================================================================*/

/* Google test and Google mock frameworks. */
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#ifndef DBG_STRING
/*! This is too often undefined in the source file to include, so added it here. */
#define DBG_STRING "\r\n %s, %d: "    /* prefix string for all debug information      */
#endif /* DBG_STRING */

/* Mocks */
#include "../mocks/mocked_foobar_extern.h"

/* Testing */
extern "C" {
#include "stzedn_typedefs.h"
#include "foobar.h"
}

using ::testing::ElementsAre;
using ::testing::Return;
using ::testing::_;

/*==============================================================================
                          GLOBAL VARIABLE DECLARATIONS
==============================================================================*/

/*==============================================================================
                                CLASS DEFINITION
==============================================================================*/
class testSuite_sample : public testing::Test
{
  public:
	MockExtern mock_extern;
  void SetUp()
  {
    /*
     * Call your initialisation function and whatever you need here.
     * e.g. reset static variables from the tested module, init rand(), ...
     */
	  setMockExternPointer(&mock_extern);
  }
  void TearDown()
  {
	  setMockExternPointer(NULL);
  }
};

/*==============================================================================
                                   UNIT TESTS
==============================================================================*/

/*============================================================================*/
/*
 * @TestSuite              testSuite_sample
 * @TestName               foo_sumTest
 * @FunctionUnderTest      foo_sum()
 */
/*============================================================================*/
TEST_F(testSuite_sample, foo_sumTest) {
	uint8_t pc_data[] = {1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U};
	uint16_t i_return;

	/* Calling function under test */
	i_return = foo_sum(pc_data, 10U);

	/* Expecting result */
	EXPECT_EQ(55U, i_return);
} /* foo_sumTest */

/*============================================================================*/
/*
 * @TestSuite              testSuite_sample
 * @TestName               foo_createFiboTest
 * @FunctionUnderTest      foo_createFibo()
 */
/*============================================================================*/
TEST_F(testSuite_sample, foo_createFiboTest) {
	uint8_t pc_dataFibo[10U];

	/* Calling function under test */
	foo_createFibo(pc_dataFibo, 10U);

	/* Expecting result */
	EXPECT_THAT(pc_dataFibo, ElementsAre(1U, 1U, 2U, 3U, 5U, 8U, 13U, 21U, 34U, 55U));
} /* foo_createFiboTest */

/*============================================================================*/
/*
 * @TestSuite              testSuite_sample
 * @TestName               foo_failingTest
 * @FunctionUnderTest      -
 */
/*============================================================================*/
TEST_F(testSuite_sample, foo_failingTest) {
	FAIL() << " - This test will just simply fails";
} /* foo_failingTest */

/*============================================================================*/
/*
 * @TestSuite              testSuite_sample
 * @TestName               foo_callingExternalTest
 * @FunctionUnderTest      foo_getRandomNumber()
 */
/*============================================================================*/
TEST_F(testSuite_sample, foo_callingExternalTest) {
	/* Expecting that 'foo_extern_getNumber()' will be called */
	EXPECT_CALL(mock_extern, foo_extern_getNumber(0U))
			   .WillOnce(Return(0xFFU));

	/* Calling function under test */
	EXPECT_EQ(0xFFU, foo_getNumber(0U));
} /* foo_callingExternalTest */
