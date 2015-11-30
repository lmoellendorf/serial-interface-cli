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
  std::string hex_string = "5AA55AF00F";
  uint8_t hex_array_test[] = { 0x5a, 0xa5, 0x5a, 0xf0, 0x0f };
  StringHex hex;
  std::vector<uint8_t> hex_vector;
  ASSERT_THAT ( hex.HexStringToBinary ( hex_string, hex_vector ), testing::ElementsAreArray ( hex_array_test ) );
}

TEST_F ( TestStringHex, UnevenHexStringToBinaryTest )
{
  std::string hex_string = "5AA55AF09";
  uint8_t hex_array_test[] = { 0x5a, 0xa5, 0x5a, 0xf0, 0x09 };
  StringHex hex;
  std::vector<uint8_t> hex_vector;
  ASSERT_THAT ( hex.HexStringToBinary ( hex_string, hex_vector ), testing::ElementsAreArray ( hex_array_test ) );
}

TEST_F ( TestStringHex, LowerCaseHexStringToBinaryTest )
{
  std::string hex_string = "5aa55af00f";
  uint8_t hex_array_test[] = { 0x5a, 0xa5, 0x5a, 0xf0, 0x0f };
  StringHex hex;
  std::vector<uint8_t> hex_vector;
  ASSERT_THAT ( hex.HexStringToBinary ( hex_string, hex_vector ), testing::ElementsAreArray ( hex_array_test ) );
}

TEST_F ( TestStringHex, MixedCaseHexStringToBinaryTest )
{
  std::string hex_string = "5aA55aF00f";
  uint8_t hex_array_test[] = { 0x5a, 0xa5, 0x5a, 0xf0, 0x0f };
  StringHex hex;
  std::vector<uint8_t> hex_vector;
  ASSERT_THAT ( hex.HexStringToBinary ( hex_string, hex_vector ), testing::ElementsAreArray ( hex_array_test ) );
}

TEST_F ( TestStringHex, PrefixedHexStringToBinaryTest )
{
  std::string hex_string = "0x5AA55AF00F";
  uint8_t hex_array_test[] = { 0x5a, 0xa5, 0x5a, 0xf0, 0x0f };
  StringHex hex;
  std::vector<uint8_t> hex_vector;
  ASSERT_THAT ( hex.HexStringToBinary ( hex_string, hex_vector ), testing::ElementsAreArray ( hex_array_test ) );
}

TEST_F ( TestStringHex, UppercasePrefixedHexStringToBinaryTest )
{
  std::string hex_string = "0X5AA55AF00F";
  uint8_t hex_array_test[] = { 0x5a, 0xa5, 0x5a, 0xf0, 0x0f };
  StringHex hex;
  std::vector<uint8_t> hex_vector;
  ASSERT_THAT ( hex.HexStringToBinary ( hex_string, hex_vector ), testing::ElementsAreArray ( hex_array_test ) );
}

TEST_F ( TestStringHex, SpaceSeparatedHexStringToBinaryTest )
{
  std::string hex_string = "5A A5 5A F0 0F";
  uint8_t hex_array_test[] = { 0x5a, 0xa5, 0x5a, 0xf0, 0x0f };
  StringHex hex;
  std::vector<uint8_t> hex_vector;
  ASSERT_THAT ( hex.HexStringToBinary ( hex_string, hex_vector ), testing::ElementsAreArray ( hex_array_test ) );
}

TEST_F ( TestStringHex, PrefixedSpaceSeparatedHexStringToBinaryTest )
{
  std::string hex_string = "0x5A 0xA5 0x5A 0xF0 0x0F";
  uint8_t hex_array_test[] = { 0x5a, 0xa5, 0x5a, 0xf0, 0x0f };
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
  std::string hex_string = "5AA55AF00FINVALID";
  uint8_t hex_array_test[] = { 0x5a, 0xa5, 0x5a, 0xf0, 0x0f };
  StringHex hex;
  std::vector<uint8_t> hex_vector;
  ASSERT_THAT ( hex.HexStringToBinary ( hex_string, hex_vector ), testing::ElementsAreArray ( hex_array_test ) );
}

TEST_F ( TestStringHex, PartlyInvalidSpaceSeparatedHexStringToBinaryTest )
{
  std::string hex_string = "5A A5 5A F0 0F INVALID";
  uint8_t hex_array_test[] = { 0x5a, 0xa5, 0x5a, 0xf0, 0x0f };
  StringHex hex;
  std::vector<uint8_t> hex_vector;
  ASSERT_THAT ( hex.HexStringToBinary ( hex_string, hex_vector ), testing::ElementsAreArray ( hex_array_test ) );
}

TEST_F ( TestStringHex, BinaryToHexStringTest )
{
  std::vector<uint8_t>  hex_vector = { 0x5a, 0xa5, 0x5a, 0xf0, 0x0f };
  std::string hex_string_test = "5A A5 5A F0 0F";
  StringHex hex;
  std::string hex_string;
  ASSERT_THAT ( hex.BinaryToHexString ( hex_vector, hex_string ), testing::StrEq(hex_string_test) );
}
