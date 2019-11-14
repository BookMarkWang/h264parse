#include "trace.hpp"
#include "nalunit.hpp"

void NalUnit::add_head(uint8_t head)
{
	m_data.forbidden_zero_bit = (head & 0x80) >> 7;
	m_data.nal_ref_idc = (head & 0x60) >> 5;
	m_data.nal_unit_type = head & 0x1F;
	m_nal_head = true;
}

void NalUnit::add_bytes(uint8_t data)
{
	if(!m_nal_head)
	{
		add_head(data);
	}
	else
	{
		if(data == 0x00 && !m_reach_threshold && m_data.rbsp_data.size() && m_data.rbsp_data.back() == 0x00)
		{
			m_reach_threshold = true;
			m_data.rbsp_data.push_back(data);
		}
		else if(data == 0x03 && m_reach_threshold)
		{
			m_reach_threshold = false;
		}
		else
		{
			m_data.rbsp_data.push_back(data);
			m_reach_threshold = false;
		}
	}
}

void NalUnit::add_bytes(uint8_t* data, uint32_t length)
{
	for(uint32_t i = 0; i < length; i++)
	{
		add_bytes(*(data+i));
	}
}

void NalUnit::add_bytes(std::vector<uint8_t>& data)
{
	for(const auto &item: data)
	{
		add_bytes(item);
	}
}

std::vector<uint8_t> NalUnit::get_rbsp()
{
	return m_data.rbsp_data;
}

uint8_t NalUnit::get_nal_forbidden_bit()
{
	return m_data.forbidden_zero_bit;
}

uint8_t NalUnit::get_nal_ref_idc()
{
	return m_data.nal_ref_idc;
}

uint8_t NalUnit::get_nal_unit_type()
{
	return m_data.nal_unit_type;
}

void NalUnit::parse(std::vector<std::shared_ptr<SPS>>& sps, std::vector<std::shared_ptr<PPS>>& pps)
{
	switch(m_data.nal_unit_type)
	{
	case 7:
	{
		std::shared_ptr<SPS> tmp = std::make_shared<SPS>(m_data);
		sps.push_back(tmp);
		m_rbsp = tmp;
		m_rbsp->parse();
		break;
	}
	case 8:
	{
		std::shared_ptr<PPS> tmp = std::make_shared<PPS>(m_data, sps);
		pps.push_back(tmp);
		m_rbsp = tmp;
		m_rbsp->parse();
		break;
	}
	default:
		TRACE_WARNING("Unhandled branch!");
		break;
	}
}

void NalUnit::clear()
{
	m_data.forbidden_zero_bit = 0;
	m_data.nal_ref_idc = 0;
	m_data.nal_unit_type = 0;
	m_data.rbsp_data.clear();
	m_rbsp.reset();
	m_reach_threshold = false;
	m_nal_head = false;
}

std::ostream& operator<<(std::ostream& os, const NalUnit& nal_unit)
{
	os << "fobidden_bit:" << static_cast<unsigned>(nal_unit.m_data.forbidden_zero_bit) << " "
		<< "ref idc:" << static_cast<unsigned>(nal_unit.m_data.nal_ref_idc) << " "
		<< "unit type:" << static_cast<unsigned>(nal_unit.m_data.nal_unit_type);
	if(nal_unit.m_rbsp)
	{
		boost::property_tree::write_json(os, nal_unit.m_rbsp->get_json_value());
	}
	return os;
}
