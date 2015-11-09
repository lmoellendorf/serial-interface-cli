#include "sf_teststringhex.h"

/* Google test and Google mock frameworks. */
#include <gtest/gtest.h>

/* Testing */
#include "sf_stringhex.h"


TestStringHex::TestStringHex()
{

}

TestStringHex::~TestStringHex()
{

}

void TestStringHex::SetUp()
{
  testing::Test::SetUp();
}

void TestStringHex::TearDown()
{
  testing::Test::TearDown();
}


TEST_F ( TestStringHex, HexStringToBinaryTest )
{
  FAIL() << " - Test not implemented yet";
}

TEST_F ( TestStringHex, BinaryToHexStringTest )
{
  FAIL() << " - Test not implemented yet";
}
