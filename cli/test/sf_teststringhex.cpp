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
  std::string hex_string = "55AA55FF00";
  uint8_t hex_array_test[] = { 0x55, 0xaa, 0x55, 0xff, 0x00 };
  StringHex hex;
  std::vector<uint8_t> hex_vector;
  ASSERT_THAT ( hex.HexStringToBinary ( hex_string, hex_vector ), testing::ElementsAreArray ( hex_array_test ) );
}

TEST_F ( TestStringHex, UnevenHexStringToBinaryTest )
{
  std::string hex_string = "55AA55FF9";
  uint8_t hex_array_test[] = { 0x55, 0xaa, 0x55, 0xff, 0x09 };
  StringHex hex;
  std::vector<uint8_t> hex_vector;
  ASSERT_THAT ( hex.HexStringToBinary ( hex_string, hex_vector ), testing::ElementsAreArray ( hex_array_test ) );
}

TEST_F ( TestStringHex, LowerCaseHexStringToBinaryTest )
{
  std::string hex_string = "55aa55ff00";
  uint8_t hex_array_test[] = { 0x55, 0xaa, 0x55, 0xff, 0x00 };
  StringHex hex;
  std::vector<uint8_t> hex_vector;
  ASSERT_THAT ( hex.HexStringToBinary ( hex_string, hex_vector ), testing::ElementsAreArray ( hex_array_test ) );
}

TEST_F ( TestStringHex, MixedCaseHexStringToBinaryTest )
{
  std::string hex_string = "55aA55Ff00";
  uint8_t hex_array_test[] = { 0x55, 0xaa, 0x55, 0xff, 0x00 };
  StringHex hex;
  std::vector<uint8_t> hex_vector;
  ASSERT_THAT ( hex.HexStringToBinary ( hex_string, hex_vector ), testing::ElementsAreArray ( hex_array_test ) );
}

TEST_F ( TestStringHex, PrefixedHexStringToBinaryTest )
{
  std::string hex_string = "0x55AA55FF00";
  uint8_t hex_array_test[] = { 0x55, 0xaa, 0x55, 0xff, 0x00 };
  StringHex hex;
  std::vector<uint8_t> hex_vector;
  ASSERT_THAT ( hex.HexStringToBinary ( hex_string, hex_vector ), testing::ElementsAreArray ( hex_array_test ) );
}

TEST_F ( TestStringHex, SpaceSeparatedHexStringToBinaryTest )
{
  std::string hex_string = "55 AA 55 FF 00";
  uint8_t hex_array_test[] = { 0x55, 0xaa, 0x55, 0xff, 0x00 };
  StringHex hex;
  std::vector<uint8_t> hex_vector;
  ASSERT_THAT ( hex.HexStringToBinary ( hex_string, hex_vector ), testing::ElementsAreArray ( hex_array_test ) );
}

TEST_F ( TestStringHex, PrefixedSpaceSeparatedHexStringToBinaryTest )
{
  std::string hex_string = "0x55 0xAA 0x55 0xFF 0x00";
  uint8_t hex_array_test[] = { 0x55, 0xaa, 0x55, 0xff, 0x00 };
  StringHex hex;
  std::vector<uint8_t> hex_vector;
  ASSERT_THAT ( hex.HexStringToBinary ( hex_string, hex_vector ), testing::ElementsAreArray ( hex_array_test ) );
}

TEST_F ( TestStringHex, InvalidHexStringToBinaryTest )
{
  std::string hex_string = "INVALID";
  StringHex hex;
  std::vector<uint8_t> hex_vector;
  ASSERT_THAT ( hex.HexStringToBinary ( hex_string, hex_vector ), testing::IsEmpty() );
}

TEST_F ( TestStringHex, PartlyInvalidHexStringToBinaryTest )
{
  std::string hex_string = "55AA55FF00INVALID";
  uint8_t hex_array_test[] = { 0x55, 0xaa, 0x55, 0xff, 0x00 };
  StringHex hex;
  std::vector<uint8_t> hex_vector;
  ASSERT_THAT ( hex.HexStringToBinary ( hex_string, hex_vector ), testing::ElementsAreArray ( hex_array_test ) );
}

TEST_F ( TestStringHex, PartlyInvalidSpaceSeparatedHexStringToBinaryTest )
{
  std::string hex_string = "55 AA 55 FF 00 INVALID";
  uint8_t hex_array_test[] = { 0x55, 0xaa, 0x55, 0xff, 0x00 };
  StringHex hex;
  std::vector<uint8_t> hex_vector;
  ASSERT_THAT ( hex.HexStringToBinary ( hex_string, hex_vector ), testing::ElementsAreArray ( hex_array_test ) );
}

TEST_F ( TestStringHex, BinaryToHexStringTest )
{
  FAIL() << " - Test not implemented yet";
  uint8_t test_hex[] = { 0x55, 0xaa, 0x55, 0xff, 0x00 };
}
