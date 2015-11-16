#include "sf_teststringhex.h"

/* Google test and Google mock frameworks. */
#include <gtest/gtest.h>
#include <gmock/gmock.h>

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
  std::string hex_string = "0x55 0xAA 0x55 0xFF 0x00";
  uint8_t hex_array_test[] = { 0x55, 0xaa, 0x55, 0xff, 0x00 };
  StringHex hex;
  ASSERT_THAT ( hex.HexStringToBinary ( hex_string ), testing::ElementsAreArray ( hex_array_test ) ) << "eh?";
}

TEST_F ( TestStringHex, BinaryToHexStringTest )
{
  FAIL() << " - Test not implemented yet";
}
