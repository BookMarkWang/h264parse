#ifndef __NAL_UNIT_HPP__
#define __NAL_UNIT_HPP__

#include <cstdint>
#include <vector>
#include <iostream>
#include <memory>
#include "rbsp.hpp"

class NalUnit
{
public:
	void add_head(uint8_t head);
	void add_bytes(uint8_t data);
	void add_bytes(uint8_t* data, uint32_t length);
	void add_bytes(std::vector<uint8_t>& data);
	void clear();
	std::vector<uint8_t> get_rbsp();
	uint8_t get_nal_forbidden_bit();
	uint8_t get_nal_ref_idc();
	uint8_t get_nal_unit_type();
	void parse(std::vector<std::shared_ptr<SPS>>& sps, std::vector<std::shared_ptr<PPS>>& pps);
	friend std::ostream& operator<<(std::ostream& os, const NalUnit& nal_unit);
private:
	NalUnitData m_data;
	bool m_reach_threshold;
	bool m_nal_head;
	std::shared_ptr<Rbsp> m_rbsp;
};

#endif //__NAL_UNIT_HPP__
