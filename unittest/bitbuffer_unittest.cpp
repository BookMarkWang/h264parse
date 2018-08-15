#include <gtest/gtest.h>
#include "bitbuffer.hpp"

TEST(BitBuffer, DefaultConstructor)
{
	BitBuffer buffer;
	EXPECT_FALSE(buffer.has_bits());
	EXPECT_FALSE(buffer.has_bytes());
}

TEST(BitBuffer, bytes_handle)
{
	BitBuffer buffer;
	std::deque<uint8_t> data;
	EXPECT_EQ(E_BITBUFFER_EMPTY, buffer.read_bytes(data, 1));
	BitBuffer buffer1(std::deque<std::bitset<8>>{std::bitset<8>(0x01), std::bitset<8>(0x02), std::bitset<8>(0x03)});
	EXPECT_EQ(E_BITBUFFER_OK, buffer1.read_bytes(data, 2));
	ASSERT_EQ(2, data.size());
	EXPECT_EQ(0x01, data.front());
	EXPECT_EQ(0x02, data.back());
	data.clear();
	EXPECT_EQ(E_BITBUFFER_NOT_ENOUGH_BYTES, buffer1.read_bytes(data, 2));
	ASSERT_EQ(1, data.size());
	EXPECT_EQ(0x03, data.front());
	data.clear();
	EXPECT_FALSE(buffer1.has_bits());
	EXPECT_FALSE(buffer1.has_bytes());
	buffer1.add_byte(0x11);
	EXPECT_EQ(E_BITBUFFER_OK, buffer1.read_bytes(data, 1));
	ASSERT_EQ(1, data.size());
	EXPECT_EQ(0x11, data.front());
}

TEST(BitBuffer, bits_handle)
{
	BitBuffer buffer;
	std::string data;
	EXPECT_EQ(E_BITBUFFER_EMPTY, buffer.read_bits(data, 1));
	buffer.add_bits(std::string("1010101011111111"));
	buffer.add_bits(std::string("0101"));
	EXPECT_EQ(E_BITBUFFER_OK, buffer.read_bits(data, 1));
	EXPECT_STREQ("1", data.c_str());
	data.clear();
	EXPECT_EQ(E_BITBUFFER_OK, buffer.read_bits(data, 2));
	EXPECT_STREQ("01", data.c_str());
	data.clear();
	EXPECT_EQ(E_BITBUFFER_OK, buffer.read_bits(data, 15));
	EXPECT_STREQ("010101111111101", data.c_str());
	data.clear();
	EXPECT_EQ(E_BITBUFFER_NOT_ENOUGH_BITS, buffer.read_bits(data, 4));
	EXPECT_STREQ("01", data.c_str());
	data.clear();
	buffer.add_bit(1);
	buffer.add_bit(0);
	EXPECT_EQ(E_BITBUFFER_OK, buffer.read_bits(data, 2));
	EXPECT_STREQ("10", data.c_str());
	data.clear();
}

TEST(BitBuffer, bits_bytes_mix)
{
	BitBuffer buffer(std::deque<std::bitset<8>>{std::bitset<8>(0x01), std::bitset<8>(0x02), std::bitset<8>(0x03)});
	std::string bit_datas;
	std::deque<uint8_t> byte_datas;
	buffer.add_bits(std::string("01010101"));
	buffer.add_byte(0xFF);
	buffer.add_bits(std::string("111"));
	EXPECT_EQ(E_BITBUFFER_OK, buffer.read_bits(bit_datas, 4));
	EXPECT_STREQ("0000", bit_datas.c_str());
	bit_datas.clear();
	EXPECT_EQ(E_BITBUFFER_OK, buffer.read_bytes(byte_datas, 4));
	ASSERT_EQ(4, byte_datas.size());
	EXPECT_EQ(0x10, byte_datas.front());
	EXPECT_EQ(0x5F, byte_datas.back());
	byte_datas.clear();
	EXPECT_EQ(E_BITBUFFER_NOT_ENOUGH_BYTES, buffer.read_bytes(byte_datas, 1));
	EXPECT_EQ(E_BITBUFFER_OK, buffer.read_bits(bit_datas, 6));
	EXPECT_STREQ("111111", bit_datas.c_str());
	bit_datas.clear();
	buffer.add_bits(std::string("0100101"));
	EXPECT_TRUE(buffer.has_bits());
	EXPECT_TRUE(buffer.has_bytes());
	EXPECT_EQ(E_BITBUFFER_OK, buffer.read_bytes(byte_datas, 1));
	ASSERT_EQ(1, byte_datas.size());
	EXPECT_EQ(0xA5, byte_datas.front());
	byte_datas.clear();
}
