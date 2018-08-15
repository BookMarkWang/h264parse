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

class SPS : public Rbsp
{
public:
	SPS(std::vector<uint8_t> data);
	eRbspState parse();
	boost::property_tree::ptree get_json_value();
private:
	struct HrdParam
	{
		//HrdParam();
		boost::property_tree::ptree get_json_value();
		uint32_t cpb_cnt_minus1;//ue(v)
		uint8_t bit_rate_scale;//u(4)
		uint8_t cpb_size_scale;//u(4)
		std::vector<uint32_t> bit_rate_value_minus1;//ue(v)
		std::vector<uint32_t> cpb_size_value_minus1;//ue(v)
		std::vector<bool> cbr_flag;//u(1)
		uint8_t initial_cpb_removal_delay_length_minus1;//u(5)
		uint8_t cpb_removal_delay_length_minus1;//u(5)
		uint8_t dpb_output_delay_length_minus1;//u(5)
		uint8_t time_offset_length;//u(5)
	};
	struct VuiParam
	{
		//VuiParam();
		boost::property_tree::ptree get_json_value();
		uint8_t aspect_ratio_info_present_flag;//u(1)
		uint8_t aspect_ratio_idc;//u(8)
		uint16_t sar_width;//u(16)
		uint16_t sar_height;//u(16)
		uint8_t overscan_info_present_flag;//u(1)
		uint8_t overscan_appropriate_flag;//u(1)
		uint8_t video_signal_type_present_flag;//u(1)
		uint8_t video_format;//u(3)
		uint8_t video_full_range_flag;//u(1)
		uint8_t colour_description_present_flag;//u(1)
		uint8_t colour_primaries;//u(8)
		uint8_t transfer_characteristics;//u(8)
		uint8_t matrix_coefficients;//u(8)
		uint8_t chroma_loc_info_present_flag;//u(1)
		uint32_t chroma_sample_loc_type_top_field;//ue(v)
		uint32_t chroma_sample_loc_type_bottom_field;//ue(v)
		uint8_t timing_info_present_flag;//u(1)
		uint32_t num_units_in_tick;//u(32)
		uint32_t time_scale;//u(32)
		uint8_t fixed_frame_rate_flag;//u(1)
		uint8_t nal_hrd_parameters_present_flag;//u(1)
		HrdParam nal_hrd_params;
		uint8_t vcl_hrd_parameters_present_flag;//u(1)
		HrdParam vcl_hrd_params;
		uint8_t low_delay_hrd_flag;//u(1)
		uint8_t pic_struct_present_flag;//u(1)
		uint8_t bitstream_restriction_flag;//u(1)
		uint8_t motion_vectors_over_pic_boundaries_flag;//u(1)
		uint32_t max_bytes_per_pic_denom;//ue(v)
		uint32_t max_bits_per_mb_denom;//ue(v)
		uint32_t log2_max_mv_length_horizontal;//ue(v)
		uint32_t log2_max_mv_length_vertical;//ue(v)
		uint32_t num_reorder_frames;//ue(v)
		uint32_t max_dec_frame_buffering;//ue(v)
	};

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

private:
	uint8_t m_profile_idc;//u(8)
	uint8_t m_constraint_set0_flag;//u(1)
	uint8_t m_constraint_set1_flag;//u(1)
	uint8_t m_constraint_set2_flag;//u(1)
	uint8_t m_constraint_set3_flag;//u(1)
	uint8_t m_reserved_zero_4bits;//u(4)
	uint8_t m_level_idc;//u(8)
	uint32_t m_sps_id;//ue(v)
	uint32_t m_chroma_format_idc;//ue(v)
	uint8_t m_residual_colour_transform_flag;//u(1)
	uint32_t m_bit_depth_luma_minus8;//ue(v)
	uint32_t m_bit_depth_chroma_minus8;///ue(v)
	uint8_t m_qpprime_y_zero_transform_bypass_flag;//u(1)
	uint8_t m_seq_scaling_matrix_present_flag;//u(1)
	std::array<uint8_t, 8> m_seq_scaling_list_present_flag;//u(1) for each
	std::array<std::array<uint8_t, 16>, 6> m_scaling_list_4x4;
	std::array<uint8_t, 6> m_use_default_scaling_matix_4x4_flag;
	std::array<std::array<uint8_t, 64>, 6> m_scaling_list_8x8;
	std::array<uint8_t, 6> m_use_default_scaling_matix_8x8_flag;
	uint32_t m_log2_max_frame_num_minus4;//ue(v)
	uint32_t m_pic_order_cnt_type;//ue(v)
	uint32_t m_log2_max_pic_order_cnt_lsb_minus4;//ue(v)
	uint8_t m_delta_pic_order_always_zero_flag;//u(1)
	int32_t m_offset_for_non_ref_pic;//se(v)
	int32_t m_offset_for_top_to_bottom_field;//se(v)
	uint32_t m_num_ref_frames_in_pic_order_cnt_cycle;//ue(v)
	std::vector<int32_t> m_offset_for_ref_frame;//se(v)
	uint32_t m_num_ref_frames;//ue(v)
	uint8_t m_gaps_in_frame_num_value_allowed_flag;//u(1)
	uint32_t m_pic_width_in_mbs_minus1;//ue(v)
	uint32_t m_pic_height_in_map_units_minus1;//ue(v)
	uint8_t m_frame_mbs_only_flag;//u(1)
	uint8_t m_mb_adaptive_frame_field_flag;//u(1)
	uint8_t m_direct_8x8_inference_flag;//u(1)
	uint8_t m_frame_cropping_flag;//u(1)
	uint32_t m_frame_crop_left_offset;//ue(v)
	uint32_t m_frame_crop_right_offset;//ue(v)
	uint32_t m_frame_crop_top_offset;//ue(v)
	uint32_t m_frame_crop_bottom_offset;//ue(v)
	uint8_t m_vui_parameters_present_flag;//u(1)
	VuiParam m_vui;
private:
	template <std::size_t SIZE>
	eRbspState parse_scaling_list(std::array<uint8_t, SIZE>& scaling_list, uint8_t& matix_flag);
	eRbspState parse_vui_parameters();
	eRbspState parse_hrd_parameters(HrdParam& hrd_param);
};

class PPS : public Rbsp
{
public:
	PPS(std::vector<uint8_t> data);
	eRbspState parse();
	boost::property_tree::ptree get_json_value();
private:
	uint32_t pic_parameter_set_id;//ue(v)
	uint32_t seq_parameter_set_id;//ue(v)
	uint8_t entropy_coding_mode_flag;//u(1)
	uint8_t pic_order_present_flag;//u(1)
	uint32_t num_slice_groups_minus1;//ue(v)
	uint32_t slice_group_map_type;//ue(v)
	std::vector<uint32_t> run_length_minus1;//ue(v)
	std::vector<uint32_t> top_left;//ue(v)
	std::vector<uint32_t> bottom_right;//ue(v)
	uint8_t slice_group_change_direction_flag;//u(1)
	uint32_t slice_group_change_rate_minus1;//ue(v)
	uint32_t pic_size_in_map_units_minus1;//ue(v)
	std::vector<uint32_t> slice_group_id;//u(v)
	uint32_t num_ref_idx_l0_active_minus1;//ue(v)
	uint32_t num_ref_idx_l1_active_minus1;//ue(v)
	uint8_t weighted_pred_flag;//u(1)
	uint8_t weighted_bipred_idc;//u(2)
	int32_t pic_init_qp_minus26;//se(v)
	int32_t pic_init_qs_minus26;//se(v)
	int32_t chroma_qp_index_offset;//se(v)
	uint8_t deblocking_filter_control_present_flag;//u(1)
	uint8_t constrained_intra_pred_flag;//u(1)
	uint8_t redundant_pic_cnt_present_flag;//u(1)
	bool more_data;
	uint8_t transform_8x8_mode_flag;//u(1)
	uint8_t pic_scaling_matrix_present_flag;//u(1)
	std::vector<bool> pic_scaling_list_present_flag;//u(1)
	int32_t second_chroma_qp_index_offset;//se(v)
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
