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

struct NalUnitData
{
	uint8_t forbidden_zero_bit;
	uint8_t nal_ref_idc;
	uint8_t nal_unit_type;
	std::vector<uint8_t> rbsp_data;
};

class Rbsp
{
public:
	Rbsp(NalUnitData data);
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
	template <std::size_t SIZE>
	eRbspState parse_scaling_list(std::array<uint8_t, SIZE>& scaling_list, uint8_t& matix_flag);
protected:
	BitBuffer m_buffer;
	NalUnitData m_nal_data;
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
	friend class PPS;
	friend class IDR;
	SPS(NalUnitData data);
	eRbspState parse();
	boost::property_tree::ptree get_json_value();
private:
	std::shared_ptr<SPSData> m_data;
private:
	eRbspState parse_vui_parameters();
	eRbspState parse_hrd_parameters(HrdParam& hrd_param);
};

struct PPSData;
class PPS : public Rbsp
{
public:
	friend class IDR;
	PPS(NalUnitData data, std::vector<std::shared_ptr<SPS>> sps);
	eRbspState parse();
	boost::property_tree::ptree get_json_value();
private:
	std::shared_ptr<PPSData> m_data;
	std::vector<std::shared_ptr<SPS>> m_sps;
};
/*
class SEI : public Rbsp
{
public:
	SEI(std::vector<uint8_t> data);
	eRbspState parse();
private:
	
}*/

typedef enum _eH264SliceType
{
  H264_P_SLICE    = 0,
  H264_B_SLICE    = 1,
  H264_I_SLICE    = 2,
  H264_SP_SLICE   = 3,
  H264_SI_SLICE   = 4,
  H264_S_P_SLICE  = 5,
  H264_S_B_SLICE  = 6,
  H264_S_I_SLICE  = 7,
  H264_S_SP_SLICE = 8,
  H264_S_SI_SLICE = 9
}eH264SliceType;

struct SliceHeader;
struct RefPicListReordering;
struct PredWeightTable;
struct DecRefPicMarking;

class IDR : public Rbsp
{
public:
	IDR(NalUnitData data, std::vector<std::shared_ptr<PPS>> pps);
	eRbspState parse();
	boost::property_tree::ptree get_json_value();
	eRbspState slice_layer_without_partitioning_rbsp();
	eRbspState slice_header(std::shared_ptr<SliceHeader> header);
	eRbspState slice_data();
	eRbspState rbsp_slice_trailing_bits();
	eRbspState ref_pic_list_reordering(std::shared_ptr<SliceHeader> header);
	eRbspState pred_weight_table(std::shared_ptr<SliceHeader> header);
	eRbspState dec_ref_pic_marking(std::shared_ptr<SliceHeader> header);
private:
	std::shared_ptr<SliceHeader> m_slice_header;
	std::vector<std::shared_ptr<PPS>> m_pps;
};

#endif //__RBSP_HPP__
