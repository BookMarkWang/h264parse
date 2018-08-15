#include <iostream>
#include <deque>
#include <system_error>
#include "h264_parse.hpp"
#include "trace.hpp"

H264Parser::H264Parser()
{
	std::cout << __func__ << std::endl;
}

H264Parser::H264Parser(std::string file)
{
	std::cout << __func__ << " " << file << std::endl;
	m_stream.open(file);
}

H264Parser::~H264Parser()
{
	if(m_stream.is_open())
	{
		m_stream.close();
	}
}

void H264Parser::set_frame_buffer(uint8_t* data, uint32_t length)
{
	NalUnit nal_unit;
	nal_unit.add_bytes(data, length);
	m_nal_unit.push_back(nal_unit);	
}

void H264Parser::set_frame_stream(uint32_t pos, uint32_t length)
{
	if(!length)
	{
		return;
	}
	if(!m_stream.is_open())
	{
		TRACE_WARNING("no file opend!!");
	}
	uint8_t tmp = 0;
	if(m_stream.tellg() != pos)
	{
		m_stream.seekg(pos);
	}
	NalUnit nal_unit;
	while(m_stream >> tmp && length--)
	{
		nal_unit.add_bytes(tmp);
	}
	if(length)
	{
		TRACE_WARNING("faild to handle ");
	}
	else
	{
		m_nal_unit.push_back(nal_unit);
	}
}

void H264Parser::start()
{
	if(!m_stream.is_open())
	{
		TRACE_WARNING("no file opend!!");
		return;
	}
	NalUnit nal_unit;
	std::deque<uint8_t> start_flag;
	char data = 0x00;
	uint8_t level = 0;
	bool is_content = false;
	while(m_stream.read(&data, 1))
	{
		if(data == 0x00)
		{
			level++;
			start_flag.push_back(data);
		}
		else if(data == 0x01 && level >=2)
		{
			if(!is_content)
			{
				is_content = true;
				level = 0;
				start_flag.clear();
			}
			else
			{
				start_flag.push_back(data);
				uint8_t size = level + 1>4?4:3;
				start_flag.erase(start_flag.end() - size);
				for(auto it = start_flag.cbegin(); it != start_flag.cend(); it++)
				{
					nal_unit.add_bytes(*it);
				}
				start_flag.clear();
				level = 0;
				nal_unit.parse();
				m_nal_unit.push_back(nal_unit);
				nal_unit.clear();
			}
		}
		else if(is_content)
		{
			for(auto it = start_flag.cbegin(); it != start_flag.cend(); it++)
			{
				nal_unit.add_bytes(*it);
			}
			start_flag.clear();
			level = 0;
			nal_unit.add_bytes(data);
		}
		else
		{
			level = 0;
		}
	}
	if(m_stream.eof())
	{
		if(is_content)
		{
			for(auto it = start_flag.cbegin(); it != start_flag.cend(); it++)
			{
				nal_unit.add_bytes(*it);
			}
			start_flag.clear();
			level = 0;
			nal_unit.parse();
			m_nal_unit.push_back(nal_unit);
			nal_unit.clear();
			is_content = false;
		}
		
	}
}

std::ostream& operator<<(std::ostream& os, const H264Parser& parser)
{
	for(const auto& item:parser.m_nal_unit)
	{
		os << item << std::endl;
	}
	return os;
}
