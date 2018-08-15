#ifndef __RBSP_HPP__
#define __RBSP_HPP__

#include <vector>
#include <array>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "bitbuffer.hpp"

typedef enum eRbspState
{
	E_RBSP_STATE_OK,
	E_RBSP_STATE_INCOMPLETE,
	E_RBSP_STATE_OVER_RUN,
	E_RBSP_STATE_INVALID_ARGUMENT,
	E_RBSP_STATE_FORMAT_ERROR,
	E_RBSP_STATE_UNKNOWN
}eRbspState;

class Rbsp
{
public:
	Rbsp(std::vector<uint8_t> data);
	eRbspState read_ue(uint32_t& value);
	eRbspState read_se(int32_t& value);
	eRbspState read_me(uint32_t& value);
	//eRbspState read_bits(uint8_t& data, uint32_t size);
	//eRbspState read_bits(uint16_t& data, uint32_t size);
	template <typename T>
	eRbspState read_bits(T& data, uint32_t size);
	virtual eRbspState parse() = 0;
	virtual boost::property_tree::ptree get_json_value() = 0;
	eRbspState parse_rbsp_trailing_bits();
	bool more_rbsp_data();
protected:
	BitBuffer m_buffer;
};

struct HrdParam;
struct VuiParam;
struct SPSData;
typedef enum _eAspectRatio
{
	E_ASPECT_RATIO_UNDEFINED,
	E_ASPECT_RATIO_1_1,
	E_ASPECT_RATIO_12_11,
	E_ASPECT_RATIO_10_11,
	E_ASPECT_RATIO_16_11,
	E_ASPECT_RATIO_40_33,
	E_ASPECT_RATIO_24_11,
	E_ASPECT_RATIO_20_11,
	E_ASPECT_RATIO_32_11,
	E_ASPECT_RATIO_80_33,
	E_ASPECT_RATIO_18_11,
	E_ASPECT_RATIO_15_11,
	E_ASPECT_RATIO_64_33,
	E_ASPECT_RATIO_160_99,
	E_ASPECT_RATIO_NOT_USED,
	E_ASPECT_RATIO_EXTENDED_SAR=0xFF,
}eAspectRatio;

class SPS : public Rbsp
{
public:
	SPS(std::vector<uint8_t> data);
	eRbspState parse();
	boost::property_tree::ptree get_json_value();
private:
	std::shared_ptr<SPSData> m_data;
private:
	template <std::size_t SIZE>
	eRbspState parse_scaling_list(std::array<uint8_t, SIZE>& scaling_list, uint8_t& matix_flag);
	eRbspState parse_vui_parameters();
	eRbspState parse_hrd_parameters(HrdParam& hrd_param);
};

struct PPSData;
class PPS : public Rbsp
{
public:
	PPS(std::vector<uint8_t> data);
	eRbspState parse();
	boost::property_tree::ptree get_json_value();
private:
	std::shared_ptr<PPSData> m_data;
};
/*
class SEI : public Rbsp
{
public:
	SEI(std::vector<uint8_t> data);
	eRbspState parse();
private:
	
}*/
#endif //__RBSP_HPP__
