#include <cmath>
#include "trace.hpp"
#include "rbsp.hpp"

#define READ_BITS(value, size) \
		{\
			state = read_bits(value, size);\
			if( E_RBSP_STATE_OK != state )\
				goto error;\
		}
#define READ_EC(func, value) \
		{\
			state = func(value);\
			if( E_RBSP_STATE_OK != state )\
				goto error;\
		}

#define ADD_UINT_CAST(param, value) param.put(#value, static_cast<unsigned>(value))
#define ADD_NO_CAST(param, value) param.put(#value, value)
#define ADD_CHILD(param, value) param.add_child(#value, value)
#define ADD_ARRAY(param, value, start, end) \
		{\
			boost::property_tree::ptree tmp;\
			for(auto it = start; it != end; it++)\
			{\
				boost::property_tree::ptree tmp_node;\
				tmp_node.put("", *it);\
				tmp.push_back(std::make_pair("", tmp_node));\
			}\
			param.add_child(#value, tmp);\
		}

struct HrdParam
{
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

struct SPSData
{
	uint8_t profile_idc;//u(8)
	uint8_t constraint_set0_flag;//u(1)
	uint8_t constraint_set1_flag;//u(1)
	uint8_t constraint_set2_flag;//u(1)
	uint8_t constraint_set3_flag;//u(1)
	uint8_t reserved_zero_4bits;//u(4)
	uint8_t level_idc;//u(8)
	uint32_t seq_parameter_set_id;//ue(v)
	uint32_t chroma_format_idc;//ue(v)
	uint8_t residual_colour_transform_flag;//u(1)
	uint32_t bit_depth_luma_minus8;//ue(v)
	uint32_t bit_depth_chroma_minus8;///ue(v)
	uint8_t qpprime_y_zero_transform_bypass_flag;//u(1)
	uint8_t seq_scaling_matrix_present_flag;//u(1)
	std::array<uint8_t, 8> seq_scaling_list_present_flag;//u(1) for each
	std::array<std::array<uint8_t, 16>, 6> scaling_list_4x4;
	std::array<uint8_t, 6> use_default_scaling_matix_4x4_flag;
	std::array<std::array<uint8_t, 64>, 6> scaling_list_8x8;
	std::array<uint8_t, 6> use_default_scaling_matix_8x8_flag;
	uint32_t log2_max_frame_num_minus4;//ue(v)
	uint32_t pic_order_cnt_type;//ue(v)
	uint32_t log2_max_pic_order_cnt_lsb_minus4;//ue(v)
	uint8_t delta_pic_order_always_zero_flag;//u(1)
	int32_t offset_for_non_ref_pic;//se(v)
	int32_t offset_for_top_to_bottom_field;//se(v)
	uint32_t num_ref_frames_in_pic_order_cnt_cycle;//ue(v)
	std::vector<int32_t> offset_for_ref_frame;//se(v)
	uint32_t num_ref_frames;//ue(v)
	uint8_t gaps_in_frame_num_value_allowed_flag;//u(1)
	uint32_t pic_width_in_mbs_minus1;//ue(v)
	uint32_t pic_height_in_map_units_minus1;//ue(v)
	uint8_t frame_mbs_only_flag;//u(1)
	uint8_t mb_adaptive_frame_field_flag;//u(1)
	uint8_t direct_8x8_inference_flag;//u(1)
	uint8_t frame_cropping_flag;//u(1)
	uint32_t frame_crop_left_offset;//ue(v)
	uint32_t frame_crop_right_offset;//ue(v)
	uint32_t frame_crop_top_offset;//ue(v)
	uint32_t frame_crop_bottom_offset;//ue(v)
	uint8_t vui_parameters_present_flag;//u(1)
	VuiParam vui_param;
};

struct PPSData
{
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
	std::array<std::array<uint8_t, 16>, 6> scaling_list_4x4;
	std::array<uint8_t, 6> use_default_scaling_matix_4x4_flag;
	std::array<std::array<uint8_t, 64>, 6> scaling_list_8x8;
	std::array<uint8_t, 6> use_default_scaling_matix_8x8_flag;
	int32_t second_chroma_qp_index_offset;//se(v)
};

Rbsp::Rbsp(std::vector<uint8_t> data)
{
	std::deque<uint8_t> tmp(data.cbegin(), data.cend());
	m_buffer.add_bytes(tmp);
}

eRbspState Rbsp::read_ue(uint32_t& value)
{
	eRbspState state = E_RBSP_STATE_OK;
	int32_t leading_zero_bit = -1;
	for(uint8_t i = 0; !i || E_RBSP_STATE_OK != state; ++leading_zero_bit)
	{
		state = read_bits(i, 1);
	}
	state = read_bits(value, leading_zero_bit);
	value += std::exp2(leading_zero_bit) - 1;
	if( E_RBSP_STATE_OK != state )
	{
		value = 0;
	}
	return state;
}

eRbspState Rbsp::read_se(int32_t& value)
{
	uint32_t tmp = 0;
	eRbspState state = read_ue(tmp);
	value = std::pow(-1, tmp) * std::ceil(tmp/2);
	return state;
}

eRbspState Rbsp::read_me(uint32_t& value)
{
}

template <typename T>
eRbspState Rbsp::read_bits(T& data, uint32_t size)
{
	eBitBufferState state = m_buffer.read_bits(data, size);
	eRbspState rbsp_state = E_RBSP_STATE_OK;
	switch(state)
	{
	case E_BITBUFFER_OK:
		rbsp_state = E_RBSP_STATE_OK; 
		break;
	case E_BITBUFFER_EMPTY:
	case E_BITBUFFER_NOT_ENOUGH_BYTES:
	case E_BITBUFFER_NOT_ENOUGH_BITS:
		rbsp_state = E_RBSP_STATE_INCOMPLETE; 
		break;
	case E_BITBUFFER_INVALID_ARGUMENT:
	default:
		rbsp_state = E_RBSP_STATE_UNKNOWN;
		break;
	}
	return rbsp_state;
}

eRbspState Rbsp::parse_rbsp_trailing_bits()
{
	if(!m_buffer.has_bits())
	{
		TRACE_LOG("No rbsp trailing bits!!!");
		return E_RBSP_STATE_INCOMPLETE;
	}
	uint8_t rbsp_stop_one_bit = 0, bsp_alignment_zero_bit = 0;
	eRbspState state = read_bits(rbsp_stop_one_bit, 1);
	if(rbsp_stop_one_bit && E_RBSP_STATE_OK == state)
	{
		while(m_buffer.has_bits())
		{
			state = read_bits(bsp_alignment_zero_bit, 1);
			if(bsp_alignment_zero_bit)
			{
				return E_RBSP_STATE_OVER_RUN;
			}
		}
	}
	if(!rbsp_stop_one_bit)
	{
		return E_RBSP_STATE_OVER_RUN;
	}
	return E_RBSP_STATE_OK;
}

bool Rbsp::more_rbsp_data()
{
	BitBuffer tmp = m_buffer;
	bool ret = parse_rbsp_trailing_bits() == E_RBSP_STATE_OVER_RUN;
	m_buffer = tmp;
	return ret;
}

SPS::SPS(std::vector<uint8_t> data):Rbsp(data)
{
	m_data = std::make_shared<SPSData>();
	for(const auto & item: data)
	{
		std::cout << std::showbase << std::hex << static_cast<unsigned>(item) << " " << std::noshowbase;
	}
}

eRbspState SPS::parse()
{
	eRbspState state = E_RBSP_STATE_OK;
	READ_BITS(m_data->profile_idc, 8);
	READ_BITS(m_data->constraint_set0_flag, 1);
	READ_BITS(m_data->constraint_set1_flag, 1);
	READ_BITS(m_data->constraint_set2_flag, 1);
	READ_BITS(m_data->constraint_set3_flag, 1);
	READ_BITS(m_data->reserved_zero_4bits, 4);
	READ_BITS(m_data->level_idc, 8);
	READ_EC(read_ue, m_data->seq_parameter_set_id);
	if( 100 == m_data->profile_idc || 110 == m_data->profile_idc || 122 == m_data->profile_idc || 144 == m_data->profile_idc)
	{
		READ_EC(read_ue, m_data->chroma_format_idc);
		if(m_data->chroma_format_idc == 3)
		{
			READ_BITS(m_data->residual_colour_transform_flag, 1);
		}
		READ_EC(read_ue, m_data->bit_depth_luma_minus8);
		READ_EC(read_ue, m_data->bit_depth_chroma_minus8);
		READ_BITS(m_data->qpprime_y_zero_transform_bypass_flag, 1);
		READ_BITS(m_data->seq_scaling_matrix_present_flag, 1);
		if(m_data->seq_scaling_matrix_present_flag)
		{
			for(uint8_t i = 0; i < 8; ++i)
			{
				READ_BITS(m_data->seq_scaling_list_present_flag[i], 1);
				if(m_data->seq_scaling_list_present_flag[i])
				{
					if(i < 6)
					{
						parse_scaling_list(m_data->scaling_list_4x4[i], m_data->use_default_scaling_matix_4x4_flag[i]);
					}
					else
					{
						parse_scaling_list(m_data->scaling_list_8x8[i-6], m_data->use_default_scaling_matix_8x8_flag[i-6]);
					}
				}
			}
		}
	}
	READ_EC(read_ue, m_data->log2_max_frame_num_minus4);
	READ_EC(read_ue, m_data->pic_order_cnt_type);
	if(0 == m_data->pic_order_cnt_type)
	{
		READ_EC(read_ue, m_data->log2_max_pic_order_cnt_lsb_minus4);
	}
	else if( 1 == m_data->pic_order_cnt_type)
	{
		READ_BITS(m_data->qpprime_y_zero_transform_bypass_flag, 1);
		READ_EC(read_se, m_data->offset_for_non_ref_pic);
		READ_EC(read_se, m_data->offset_for_top_to_bottom_field);
		READ_EC(read_ue, m_data->num_ref_frames_in_pic_order_cnt_cycle);
		for(uint32_t i = 0; i < m_data->num_ref_frames_in_pic_order_cnt_cycle; ++i)
		{
			int32_t tmp = 0;
			READ_EC(read_se, tmp);
			m_data->offset_for_ref_frame.push_back(tmp);
		}
	}
	READ_EC(read_ue, m_data->num_ref_frames);
	READ_BITS(m_data->gaps_in_frame_num_value_allowed_flag, 1);
	READ_EC(read_ue, m_data->pic_width_in_mbs_minus1);
	READ_EC(read_ue, m_data->pic_height_in_map_units_minus1);
	READ_BITS(m_data->frame_mbs_only_flag, 1);
	if(!m_data->frame_mbs_only_flag)
	{
		READ_BITS(m_data->mb_adaptive_frame_field_flag, 1);
	}
	READ_BITS(m_data->direct_8x8_inference_flag, 1);
	READ_BITS(m_data->frame_cropping_flag, 1);
	if(m_data->frame_cropping_flag)
	{
		READ_EC(read_ue, m_data->frame_crop_left_offset);
		READ_EC(read_ue, m_data->frame_crop_right_offset);
		READ_EC(read_ue, m_data->frame_crop_top_offset);
		READ_EC(read_ue, m_data->frame_crop_bottom_offset);
	}
	READ_BITS(m_data->vui_parameters_present_flag, 1);
	if(m_data->vui_parameters_present_flag)
	{
		state = parse_vui_parameters();
	}
	state = parse_rbsp_trailing_bits();
	return state;
error:
	TRACE_ERROR("failed to parse!! error is %d", state);
	return state;
}

boost::property_tree::ptree SPS::get_json_value()
{
	boost::property_tree::ptree root;
	root.put("profile_idc", static_cast<uint32_t>(m_data->profile_idc));
	root.put("constraint_set0_flag", static_cast<uint32_t>(m_data->constraint_set0_flag));
	root.put("constraint_set1_flag", static_cast<uint32_t>(m_data->constraint_set1_flag));
	root.put("constraint_set2_flag", static_cast<uint32_t>(m_data->constraint_set2_flag));
	root.put("constraint_set3_flag", static_cast<uint32_t>(m_data->constraint_set3_flag));
	root.put("reserved_zero_4bits", static_cast<uint32_t>(m_data->reserved_zero_4bits));
	root.put("level_idc", static_cast<uint32_t>(m_data->level_idc));
	root.put("seq_parameter_set_id", m_data->seq_parameter_set_id);
	if( 100 == m_data->profile_idc || 110 == m_data->profile_idc || 122 == m_data->profile_idc || 144 == m_data->profile_idc)
	{
		root.put("chroma_format_idc", m_data->chroma_format_idc);
		if(m_data->chroma_format_idc == 3)
		{
			root.put("residual_colour_transform_flag", static_cast<uint32_t>(m_data->residual_colour_transform_flag));
		}
		root.put("bit_depth_luma_minus8", m_data->bit_depth_luma_minus8);
		root.put("bit_depth_chroma_minus8", m_data->bit_depth_chroma_minus8);
		root.put("qpprime_y_zero_transform_bypass_flag", static_cast<uint32_t>(m_data->qpprime_y_zero_transform_bypass_flag));
		root.put("seq_scaling_matrix_present_flag", static_cast<uint32_t>(m_data->seq_scaling_matrix_present_flag));
		if(m_data->seq_scaling_matrix_present_flag)
		{
			ADD_ARRAY(root, seq_scaling_list_present_flag, m_data->seq_scaling_list_present_flag.cbegin(), m_data->seq_scaling_list_present_flag.cend());
			ADD_ARRAY(root, use_default_scaling_matix_4x4_flag, m_data->use_default_scaling_matix_4x4_flag.cbegin(), m_data->use_default_scaling_matix_4x4_flag.cend());
			ADD_ARRAY(root, use_default_scaling_matix_8x8_flag, m_data->use_default_scaling_matix_8x8_flag.cbegin(), m_data->use_default_scaling_matix_8x8_flag.cend());
			//scaling_list_4x4
			//scaling_list_8x8
		}
	}
	root.put("log2_max_frame_num_minus4", m_data->log2_max_frame_num_minus4);
	root.put("pic_order_cnt_type", m_data->pic_order_cnt_type);
	if(0 == m_data->pic_order_cnt_type)
	{
		root.put("log2_max_pic_order_cnt_lsb_minus4", m_data->log2_max_pic_order_cnt_lsb_minus4);
	}
	else if( 1 == m_data->pic_order_cnt_type)
	{
		root.put("qpprime_y_zero_transform_bypass_flag", static_cast<uint32_t>(m_data->qpprime_y_zero_transform_bypass_flag));
		root.put("offset_for_non_ref_pic", m_data->offset_for_non_ref_pic);
		root.put("offset_for_top_to_bottom_field", m_data->offset_for_top_to_bottom_field);
		root.put("num_ref_frames_in_pic_order_cnt_cycle", m_data->num_ref_frames_in_pic_order_cnt_cycle);
		ADD_ARRAY(root, offset_for_ref_frame, m_data->offset_for_ref_frame.cbegin(), m_data->offset_for_ref_frame.cend());
	}
	root.put("num_ref_frames", m_data->num_ref_frames);
	root.put("gaps_in_frame_num_value_allowed_flag", static_cast<uint32_t>(m_data->gaps_in_frame_num_value_allowed_flag));
	root.put("pic_width_in_mbs_minus1", m_data->pic_width_in_mbs_minus1);
	root.put("pic_height_in_map_units_minus1", m_data->pic_height_in_map_units_minus1);
	root.put("frame_mbs_only_flag", static_cast<uint32_t>(m_data->frame_mbs_only_flag));
	if(!m_data->frame_mbs_only_flag)
	{
		root.put("mb_adaptive_frame_field_flag", static_cast<uint32_t>(m_data->mb_adaptive_frame_field_flag));
	}
	root.put("direct_8x8_inference_flag", static_cast<uint32_t>(m_data->direct_8x8_inference_flag));
	root.put("frame_cropping_flag", static_cast<uint32_t>(m_data->frame_cropping_flag));
	if(m_data->frame_cropping_flag)
	{
		root.put("frame_crop_left_offset", m_data->frame_crop_left_offset);
		root.put("frame_crop_right_offset", m_data->frame_crop_right_offset);
		root.put("frame_crop_top_offset", m_data->frame_crop_top_offset);
		root.put("frame_crop_bottom_offset", m_data->frame_crop_bottom_offset);
	}
	root.put("vui_parameters_present_flag", static_cast<uint32_t>(m_data->vui_parameters_present_flag));
	if(m_data->vui_parameters_present_flag)
	{
		root.add_child("vui_parameters", m_data->vui_param.get_json_value());
	}
	return root;
}


template <std::size_t SIZE>
eRbspState Rbsp::parse_scaling_list(std::array<uint8_t, SIZE>& scaling_list, uint8_t& matix_flag)
{
	eRbspState state = E_RBSP_STATE_OK;
	uint8_t last_scale = 8, next_scale = 8;
	int32_t delta_scale = 0;
	for(uint8_t i = 0; i < SIZE; ++i)
	{
		if(next_scale)
		{
			state = read_se(delta_scale);
			next_scale = (last_scale + delta_scale + 256) % 256;
			matix_flag = ( 0 == i && 0 == next_scale);
		}
		scaling_list[i] = (0 == next_scale)?last_scale:next_scale;
		last_scale = scaling_list[i];
	}
	return state;
}

eRbspState SPS::parse_vui_parameters()
{
	eRbspState state = E_RBSP_STATE_OK;
	READ_BITS(m_data->vui_param.aspect_ratio_info_present_flag, 1);
	if(m_data->vui_param.aspect_ratio_info_present_flag)
	{
		READ_BITS(m_data->vui_param.aspect_ratio_idc, 8);
		if(E_ASPECT_RATIO_EXTENDED_SAR == m_data->vui_param.aspect_ratio_idc)
		{
			READ_BITS(m_data->vui_param.sar_width, 16);
			READ_BITS(m_data->vui_param.sar_height, 16);
		}
	}
	READ_BITS(m_data->vui_param.overscan_info_present_flag, 1);
	if(m_data->vui_param.overscan_info_present_flag)
	{
		READ_BITS(m_data->vui_param.overscan_appropriate_flag, 1);
	}
	READ_BITS(m_data->vui_param.video_signal_type_present_flag, 1);
	if(m_data->vui_param.video_signal_type_present_flag)
	{
		READ_BITS(m_data->vui_param.video_format, 3);
		READ_BITS(m_data->vui_param.video_full_range_flag, 1);
		READ_BITS(m_data->vui_param.colour_description_present_flag, 1);
		if(m_data->vui_param.colour_description_present_flag)
		{
			READ_BITS(m_data->vui_param.colour_primaries, 8);
			READ_BITS(m_data->vui_param.transfer_characteristics, 8);
			READ_BITS(m_data->vui_param.matrix_coefficients, 8);
		}
	}
	READ_BITS(m_data->vui_param.chroma_loc_info_present_flag, 1);
	if(m_data->vui_param.chroma_loc_info_present_flag)
	{
		READ_EC(read_ue, m_data->vui_param.chroma_sample_loc_type_top_field);
		READ_EC(read_ue, m_data->vui_param.chroma_sample_loc_type_bottom_field);
	}
	READ_BITS(m_data->vui_param.timing_info_present_flag, 1);
	if(m_data->vui_param.timing_info_present_flag)
	{
		READ_BITS(m_data->vui_param.num_units_in_tick, 32);
		READ_BITS(m_data->vui_param.time_scale, 32);
		READ_BITS(m_data->vui_param.fixed_frame_rate_flag, 1);
	}
	READ_BITS(m_data->vui_param.nal_hrd_parameters_present_flag, 1);
	if(m_data->vui_param.nal_hrd_parameters_present_flag)
	{
		state = parse_hrd_parameters(m_data->vui_param.nal_hrd_params);
	}
	READ_BITS(m_data->vui_param.vcl_hrd_parameters_present_flag, 1);
	if(m_data->vui_param.vcl_hrd_parameters_present_flag)
	{
		state = parse_hrd_parameters(m_data->vui_param.vcl_hrd_params);
	}
	if(m_data->vui_param.nal_hrd_parameters_present_flag ||
		m_data->vui_param.vcl_hrd_parameters_present_flag)
	{
		READ_BITS(m_data->vui_param.low_delay_hrd_flag, 1);
	}
	READ_BITS(m_data->vui_param.pic_struct_present_flag, 1);
	READ_BITS(m_data->vui_param.bitstream_restriction_flag, 1);
	if(m_data->vui_param.bitstream_restriction_flag)
	{
		READ_BITS(m_data->vui_param.motion_vectors_over_pic_boundaries_flag, 1);
		READ_EC(read_ue, m_data->vui_param.max_bytes_per_pic_denom);
		READ_EC(read_ue, m_data->vui_param.max_bits_per_mb_denom);
		READ_EC(read_ue, m_data->vui_param.log2_max_mv_length_horizontal);
		READ_EC(read_ue, m_data->vui_param.log2_max_mv_length_vertical);
		READ_EC(read_ue, m_data->vui_param.num_reorder_frames);
		READ_EC(read_ue, m_data->vui_param.max_dec_frame_buffering);
	}
	return state;
error:
	TRACE_WARNING("failed to parse!! error is %d", state);
	return state;
}

eRbspState SPS::parse_hrd_parameters(HrdParam& hrd_param)
{
	eRbspState state = E_RBSP_STATE_OK;
	READ_EC(read_ue, hrd_param.cpb_cnt_minus1);
	READ_BITS(hrd_param.bit_rate_scale, 4);
	READ_BITS(hrd_param.cpb_size_scale, 4);
	for(uint32_t i = 0; i < hrd_param.cpb_cnt_minus1; ++i)
	{
		uint32_t tmp = 0;
		READ_EC(read_ue, tmp);
		hrd_param.bit_rate_value_minus1.push_back(tmp);
		tmp = 0;
		READ_EC(read_ue, tmp);
		hrd_param.cpb_size_value_minus1.push_back(tmp);
		tmp = 0;
		READ_BITS(tmp, 1);
		hrd_param.cbr_flag.push_back(tmp);
	}
	READ_BITS(hrd_param.initial_cpb_removal_delay_length_minus1, 5);
	READ_BITS(hrd_param.cpb_removal_delay_length_minus1, 5);
	READ_BITS(hrd_param.dpb_output_delay_length_minus1, 5);
	READ_BITS(hrd_param.time_offset_length, 5);
	return state;
error:
	TRACE_WARNING("failed to parse!! error is %d", state);
	return state;
}

boost::property_tree::ptree HrdParam::get_json_value()
{
	boost::property_tree::ptree root;
	ADD_NO_CAST(root, cpb_cnt_minus1);
	ADD_UINT_CAST(root, bit_rate_scale);
	ADD_UINT_CAST(root, cpb_size_scale);
	ADD_ARRAY(root, bit_rate_value_minus1, bit_rate_value_minus1.cbegin(), bit_rate_value_minus1.cend());
	ADD_ARRAY(root, cpb_size_value_minus1, cpb_size_value_minus1.cbegin(), cpb_size_value_minus1.cend());
	ADD_ARRAY(root, cbr_flag, cbr_flag.cbegin(), cbr_flag.cend());
	ADD_UINT_CAST(root, initial_cpb_removal_delay_length_minus1);
	ADD_UINT_CAST(root, cpb_removal_delay_length_minus1);
	ADD_UINT_CAST(root, dpb_output_delay_length_minus1);
	ADD_UINT_CAST(root, time_offset_length);
	return root;
}

boost::property_tree::ptree VuiParam::get_json_value()
{
	boost::property_tree::ptree root;
	ADD_UINT_CAST(root, aspect_ratio_info_present_flag);
	if(aspect_ratio_info_present_flag)
	{
		ADD_UINT_CAST(root, aspect_ratio_idc);
		if(E_ASPECT_RATIO_EXTENDED_SAR == aspect_ratio_idc)
		{
			ADD_UINT_CAST(root, sar_width);
			ADD_UINT_CAST(root, sar_height);
		}
	}
	ADD_UINT_CAST(root, overscan_info_present_flag);
	if(overscan_info_present_flag)
	{
		ADD_UINT_CAST(root, overscan_appropriate_flag);
	}
	ADD_UINT_CAST(root, video_signal_type_present_flag);
	if(video_signal_type_present_flag)
	{
		ADD_UINT_CAST(root, video_format);
		ADD_UINT_CAST(root, video_full_range_flag);
		ADD_UINT_CAST(root, colour_description_present_flag);
		if(colour_description_present_flag)
		{
			ADD_UINT_CAST(root, colour_primaries);
			ADD_UINT_CAST(root, transfer_characteristics);
			ADD_UINT_CAST(root, matrix_coefficients);
		}
	}
	ADD_UINT_CAST(root, chroma_loc_info_present_flag);
	if(chroma_loc_info_present_flag)
	{
		ADD_NO_CAST(root, chroma_sample_loc_type_top_field);
		ADD_NO_CAST(root, chroma_sample_loc_type_bottom_field);
	}
	ADD_UINT_CAST(root, timing_info_present_flag);
	if(timing_info_present_flag)
	{
		ADD_NO_CAST(root, num_units_in_tick);
		ADD_NO_CAST(root, time_scale);
		ADD_UINT_CAST(root, fixed_frame_rate_flag);
	}
	ADD_UINT_CAST(root, nal_hrd_parameters_present_flag);
	if(nal_hrd_parameters_present_flag)
	{
		root.add_child("nal_hrd_params", nal_hrd_params.get_json_value());
	}
	ADD_UINT_CAST(root, vcl_hrd_parameters_present_flag);
	if(vcl_hrd_parameters_present_flag)
	{
		root.add_child("vcl_hrd_params", vcl_hrd_params.get_json_value());
	}
	if(nal_hrd_parameters_present_flag ||
		vcl_hrd_parameters_present_flag)
	{
		ADD_UINT_CAST(root, low_delay_hrd_flag);
	}
	ADD_UINT_CAST(root, pic_struct_present_flag);
	ADD_UINT_CAST(root, bitstream_restriction_flag);
	if(bitstream_restriction_flag)
	{
		ADD_UINT_CAST(root, motion_vectors_over_pic_boundaries_flag);
		ADD_NO_CAST(root, max_bytes_per_pic_denom);
		ADD_NO_CAST(root, max_bits_per_mb_denom);
		ADD_NO_CAST(root, log2_max_mv_length_horizontal);
		ADD_NO_CAST(root, log2_max_mv_length_vertical);
		ADD_NO_CAST(root, num_reorder_frames);
		ADD_NO_CAST(root, max_dec_frame_buffering);
	}
	return root;
}

PPS::PPS(std::vector<uint8_t> data):Rbsp(data)
{
	m_data = std::make_shared<PPSData>();
	for(const auto & item: data)
	{
		std::cout << std::showbase << std::hex << static_cast<unsigned>(item) << " " << std::noshowbase;
	}
}

eRbspState PPS::parse()
{
	eRbspState state = E_RBSP_STATE_OK;
	READ_EC(read_ue, m_data->pic_parameter_set_id);
	READ_EC(read_ue, m_data->seq_parameter_set_id);
	READ_BITS(m_data->entropy_coding_mode_flag, 1);
	READ_BITS(m_data->pic_order_present_flag, 1);
	READ_EC(read_ue, m_data->num_slice_groups_minus1);
	if(m_data->num_slice_groups_minus1 > 0)
	{
		READ_EC(read_ue, m_data->slice_group_map_type);
		if(0 == m_data->slice_group_map_type)
		{
			for(uint32_t i = 0; i < m_data->num_slice_groups_minus1; ++i)
			{
				uint32_t tmp = 0;
				READ_EC(read_ue, tmp);
				m_data->run_length_minus1.push_back(tmp);
			}
		}
		else if( 2 == m_data->slice_group_map_type)
		{
			for(uint32_t i = 0; i < m_data->num_slice_groups_minus1; ++i)
			{
				uint32_t tmp = 0;
				READ_EC(read_ue, tmp);
				m_data->top_left.push_back(tmp);
				tmp = 0;
				READ_EC(read_ue, tmp);
				m_data->bottom_right.push_back(tmp);
			}
		}
		else if( 3 == m_data->slice_group_map_type || 
			4 == m_data->slice_group_map_type ||
			5 == m_data->slice_group_map_type)
		{
			READ_BITS(m_data->slice_group_change_direction_flag, 1);
			READ_EC(read_ue, m_data->slice_group_change_rate_minus1);
		}
		else if( 6 == m_data->slice_group_map_type)
		{
			READ_EC(read_ue, m_data->pic_size_in_map_units_minus1);
			uint32_t bit_num = std::ceil(std::exp2(m_data->num_slice_groups_minus1 + 1));
			for(uint32_t i = 0; i < m_data->pic_size_in_map_units_minus1; ++i)
			{
				uint32_t tmp = 0;
				READ_BITS(tmp, bit_num);
				m_data->slice_group_id.push_back(tmp);
			}
		}
	}
	READ_EC(read_ue, m_data->num_ref_idx_l0_active_minus1);
	READ_EC(read_ue, m_data->num_ref_idx_l1_active_minus1);
	READ_BITS(m_data->weighted_pred_flag, 1);
	READ_BITS(m_data->weighted_bipred_idc, 2);
	READ_EC(read_se, m_data->pic_init_qp_minus26);
	READ_EC(read_se, m_data->pic_init_qs_minus26);
	READ_EC(read_se, m_data->chroma_qp_index_offset);
	READ_BITS(m_data->deblocking_filter_control_present_flag, 1);
	READ_BITS(m_data->constrained_intra_pred_flag, 1);
	READ_BITS(m_data->redundant_pic_cnt_present_flag, 1);
	TRACE_LOG("-----------------------%d", m_buffer.get_bits_num());
	m_data->more_data = more_rbsp_data();
	TRACE_LOG("-----------------------%d", m_buffer.get_bits_num());
	if(m_data->more_data)
	{
		READ_BITS(m_data->transform_8x8_mode_flag, 1);
		READ_BITS(m_data->pic_scaling_matrix_present_flag, 1);
		if(m_data->pic_scaling_matrix_present_flag)
		{
			for(uint8_t i = 0; i < 6 + 2 * m_data->transform_8x8_mode_flag; ++i)
			{
				uint8_t tmp = 0;
				READ_BITS(tmp, 1);
				m_data->pic_scaling_list_present_flag.push_back(tmp);
				if(tmp)
				{
					if(i < 6)
					{
						parse_scaling_list(m_data->scaling_list_4x4[i], m_data->use_default_scaling_matix_4x4_flag[i]);
					}
					else
					{
						parse_scaling_list(m_data->scaling_list_8x8[i-6], m_data->use_default_scaling_matix_8x8_flag[i-6]);
					}
				}
			}
		}
		READ_EC(read_se, m_data->second_chroma_qp_index_offset);
	}
	state = parse_rbsp_trailing_bits();
	return state;
error:
	return state;
}

boost::property_tree::ptree PPS::get_json_value()
{
	boost::property_tree::ptree root;
	root.put("pic_parameter_set_id",  m_data->pic_parameter_set_id);
	root.put("seq_parameter_set_id",  m_data->seq_parameter_set_id);
	root.put("entropy_coding_mode_flag", static_cast<uint32_t>(m_data->entropy_coding_mode_flag));
	root.put("pic_order_present_flag", static_cast<uint32_t>(m_data->pic_order_present_flag));
	root.put("num_slice_groups_minus1", m_data->num_slice_groups_minus1);
	if(m_data->num_slice_groups_minus1 > 0)
	{
		boost::property_tree::ptree slice_group_map;
		slice_group_map.put("slice_group_map_type", m_data->slice_group_map_type);
		if(0 == m_data->slice_group_map_type)
		{
			ADD_ARRAY(slice_group_map, run_length_minus1, m_data->run_length_minus1.cbegin(), m_data->run_length_minus1.cend());
		}
		else if( 2 == m_data->slice_group_map_type)
		{
			ADD_ARRAY(slice_group_map, top_left, m_data->top_left.cbegin(), m_data->top_left.cend());
			ADD_ARRAY(slice_group_map, bottom_right, m_data->bottom_right.cbegin(), m_data->bottom_right.cend());
		}
		else if( 3 == m_data->slice_group_map_type || 
			4 == m_data->slice_group_map_type ||
			5 == m_data->slice_group_map_type)
		{
			slice_group_map.put("slice_group_change_direction_flag", static_cast<uint32_t>(m_data->slice_group_change_direction_flag));
			slice_group_map.put("slice_group_change_rate_minus1", m_data->slice_group_change_rate_minus1);
		}
		else if( 6 == m_data->slice_group_map_type)
		{
			slice_group_map.put("pic_size_in_map_units_minus1", m_data->pic_size_in_map_units_minus1);
			ADD_ARRAY(slice_group_map, slice_group_id, m_data->slice_group_id.cbegin(), m_data->slice_group_id.cend());
		}
		ADD_CHILD(root, slice_group_map);
	}
	root.put("num_ref_idx_l0_active_minus1", m_data->num_ref_idx_l0_active_minus1);
	root.put("num_ref_idx_l1_active_minus1", m_data->num_ref_idx_l1_active_minus1);
	root.put("weighted_pred_flag", static_cast<uint32_t>(m_data->weighted_pred_flag));
	root.put("weighted_bipred_idc", static_cast<uint32_t>(m_data->weighted_bipred_idc));
	root.put("pic_init_qp_minus26", m_data->pic_init_qp_minus26);
	root.put("pic_init_qs_minus26", m_data->pic_init_qs_minus26);
	root.put("chroma_qp_index_offset", m_data->chroma_qp_index_offset);
	root.put("deblocking_filter_control_present_flag", static_cast<uint32_t>(m_data->deblocking_filter_control_present_flag));
	root.put("constrained_intra_pred_flag", static_cast<uint32_t>(m_data->constrained_intra_pred_flag));
	root.put("redundant_pic_cnt_present_flag", static_cast<uint32_t>(m_data->redundant_pic_cnt_present_flag));
	if(m_data->more_data)
	{
		boost::property_tree::ptree more_rbsp_data;
		more_rbsp_data.put("transform_8x8_mode_flag", static_cast<uint32_t>(m_data->transform_8x8_mode_flag));
		more_rbsp_data.put("pic_scaling_matrix_present_flag", static_cast<uint32_t>(m_data->pic_scaling_matrix_present_flag));
		if(m_data->pic_scaling_matrix_present_flag)
		{
			ADD_ARRAY(more_rbsp_data, pic_scaling_list_present_flag, m_data->pic_scaling_list_present_flag.cbegin(), m_data->pic_scaling_list_present_flag.cend());
			ADD_ARRAY(root, use_default_scaling_matix_4x4_flag, m_data->use_default_scaling_matix_4x4_flag.cbegin(), m_data->use_default_scaling_matix_4x4_flag.cend());
			ADD_ARRAY(root, use_default_scaling_matix_8x8_flag, m_data->use_default_scaling_matix_8x8_flag.cbegin(), m_data->use_default_scaling_matix_8x8_flag.cend());
			//scaling_list_4x4
			//scaling_list_8x8
		}
		more_rbsp_data.put("second_chroma_qp_index_offset", m_data->second_chroma_qp_index_offset);
		ADD_CHILD(root, more_rbsp_data);
	}
	return root;
}
