#ifndef __H264_PARSER_HPP__
#define __H264_PARSER_HPP__

#include <vector>
#include <iostream>
#include <fstream>
#include "nalunit.hpp"

class H264Parser
{
public:
	H264Parser();
	H264Parser(std::string file);
	~H264Parser();
	void set_frame_buffer(uint8_t* data, uint32_t length);
	void set_frame_stream(uint32_t pos, uint32_t length);
	void start();
	friend std::ostream& operator<<(std::ostream& os, const H264Parser& parser);
private:
	std::ifstream m_stream;
	std::vector<NalUnit> m_nal_unit;
};

#endif //__H264_PARSER_HPP__
