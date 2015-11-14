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
  uint8_t *hex_array;
  size_t length = hex.HexStringToBinary ( &hex_string, &hex_array );
  std::vector<uint8_t> hex_vector;
  for ( size_t i = 0 ; i < length; i++ )
    {
      hex_vector.push_back ( hex_array[i] );
    }
  ASSERT_THAT ( hex_vector, testing::ElementsAreArray ( hex_array_test ) ) << "eh?";
}

TEST_F ( TestStringHex, BinaryToHexStringTest )
{
  FAIL() << " - Test not implemented yet";
}
