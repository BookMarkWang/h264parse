#include <gtest/gtest.h>
#include "nalunit.hpp"

#define EXPECT_VECTOR_EQUAL(v1, v2) \
		{\
			ASSERT_EQ(v1.size(), v2.size());\
			for(uint32_t i = 0; i < v1.size(); i++)\
			{\
				EXPECT_EQ(v1[i], v2[i]);\
			}\
		}

TEST(NalUnit, data_handle)
{
	NalUnit nal_unit;
	nal_unit.add_head(0xB3);
	EXPECT_EQ(0x1, nal_unit.get_nal_forbidden_bit());
	EXPECT_EQ(0x1, nal_unit.get_nal_ref_idc());
	EXPECT_EQ(0x13, nal_unit.get_nal_unit_type());
	nal_unit.clear();
	nal_unit.add_bytes(0x21);
	EXPECT_EQ(0x0, nal_unit.get_nal_forbidden_bit());
	EXPECT_EQ(0x1, nal_unit.get_nal_ref_idc());
	EXPECT_EQ(0x1, nal_unit.get_nal_unit_type());
	nal_unit.add_bytes(0x00);
	std::vector<uint8_t> tmp{0x03, 0x02, 0x00, 0x00};
	nal_unit.add_bytes(tmp);
	std::vector<uint8_t> data = nal_unit.get_rbsp();
	std::vector<uint8_t> expect = {0x00, 0x03, 0x02, 0x00, 0x00};
	EXPECT_VECTOR_EQUAL(data, expect);
	nal_unit.add_bytes(0x03);
	data = nal_unit.get_rbsp();
	EXPECT_VECTOR_EQUAL(data, expect);
	nal_unit.add_bytes(0x03);
	expect.push_back(0x03);
	data = nal_unit.get_rbsp();
	EXPECT_VECTOR_EQUAL(data, expect);
}
