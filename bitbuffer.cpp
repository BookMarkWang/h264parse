#include <iostream>
#include "trace.hpp"
#include "bitbuffer.hpp"


BitBuffer::BitBuffer(std::deque<std::bitset<8>> data):m_buffer(data),m_cache(std::bitset<8>(0)),m_cache_bits(0),m_head_cache(std::bitset<8>(0)),m_head_cache_bits(0)
{}

void BitBuffer::add_byte(uint8_t data)
{
	m_buffer.push_back(std::bitset<8>(data));
}

void BitBuffer::add_bytes(std::deque<uint8_t>& data)
{
	for(auto i : data)
	{
		m_buffer.push_back(std::bitset<8>(i));
	}
}

void BitBuffer::add_bit(bool data)
{
	m_cache = std::bitset<8>(m_cache.to_string().erase(0, 8 - (m_cache_bits++)) + (data?"1":"0"));
	if(m_cache_bits == 8)
	{
		m_buffer.push_back(m_cache);
		m_cache_bits = 0;
	}
}

void BitBuffer::add_bits(std::string data)
{
	if(data.find_first_not_of("01") != std::string::npos)
	{
		TRACE_WARNING("invalid parameter");
		return;// E_BITBUFFER_INVALID_ARGUMENT;
	}
	decltype(data.size()) length = data.size(), index = 0;
	/*for(decltype(data.size()) bits = m_cache_bits; bits < 8 && index < length; ++bits, ++index)
	{
		m_cache.set(7 - (m_cache_bits++), data[index]=='1'?1:0);
		if(m_cache_bits == 8)
		{
			m_buffer.push_back(m_cache);
			m_cache_bits = 0;
			m_cache.reset();
		}
	}*/
	if(m_cache_bits + data.length() >= 8)
	{
		m_buffer.push_back(std::bitset<8>(m_cache.to_string().erase(0, 8 - m_cache_bits) + data.substr(0, 8 - m_cache_bits)));
		index = 8 - m_cache_bits;
		m_cache_bits = 0;
		m_cache.reset();
	}
	else
	{
		m_cache = std::bitset<8>(m_cache.to_string().erase(0, 8 - m_cache_bits) + data.substr(0, 8 - m_cache_bits));
		m_cache_bits = m_cache_bits + data.length();
		index = length; 
	}
	for(; index < length; ++index)
	{
		decltype(data.size()) tmp_size = (length - index)<8?(length - index):8;
		if(tmp_size == 8)
		{
			m_buffer.push_back(std::bitset<8>(data, index, tmp_size));
		}
		else
		{
			m_cache = std::bitset<8>(data, index, tmp_size);
			m_cache_bits = tmp_size;
		}
		index+=tmp_size;
	}
}

bool BitBuffer::has_bits()
{
	return m_cache_bits || m_buffer.size() || m_head_cache_bits;
}

bool BitBuffer::has_bytes()
{
	return m_buffer.size() || (m_cache_bits + m_head_cache_bits >= 8);
}

uint32_t BitBuffer::get_bits_num()
{
	return m_cache_bits + m_head_cache_bits + m_buffer.size() * 8;
}

eBitBufferState BitBuffer::read_bytes(std::deque<uint8_t>& data, uint32_t size)
{
	if(m_cache_bits == 0 && m_head_cache_bits == 0 && m_buffer.size() == 0)
	{
		TRACE_WARNING("bit buffer is empty");
		return E_BITBUFFER_EMPTY;
	}
	decltype(m_buffer.size()) i = 0;
	for(; i < size && m_buffer.size(); i++)
	{
		if(m_head_cache_bits)
		{
			std::string cache = m_head_cache.to_string().erase(0, 8 - m_head_cache_bits);
			cache += m_buffer.front().to_string();
			m_head_cache = std::bitset<8>(cache.substr(8, m_head_cache_bits));
			cache = cache.erase(8, m_head_cache_bits);
			std::bitset<8> tmp(cache);
			data.push_back(static_cast<uint8_t>(tmp.to_ulong()));
		}
		else
		{
			data.push_back(static_cast<uint8_t>(m_buffer.front().to_ulong()));
		}
		m_buffer.pop_front();
	}
	if(i < size)
	{
		if(m_cache_bits + m_head_cache_bits < 8)
		{
			TRACE_WARNING("no enough bytes");
			return E_BITBUFFER_NOT_ENOUGH_BYTES;
		}
		std::string cache = m_head_cache.to_string().erase(0, 8 - m_head_cache_bits)
					+ m_cache.to_string().erase(0, 8 - m_cache_bits);
		m_head_cache = std::bitset<8>(cache.substr(8));
		m_head_cache_bits = m_cache_bits + m_head_cache_bits - 8;
		cache = cache.erase(8);
		std::bitset<8> tmp(cache);
		data.push_back(static_cast<uint8_t>(tmp.to_ulong()));
		m_cache.reset();
		m_cache_bits = 0;
		if(m_head_cache_bits == 8)
		{
			m_buffer.push_back(m_head_cache);
			m_head_cache.reset();
			m_head_cache_bits = 0;
		}
	}
	return E_BITBUFFER_OK;
}

eBitBufferState BitBuffer::read_bits(std::string& data, uint32_t size)
{
	if(m_cache_bits == 0 && m_head_cache_bits == 0 && m_buffer.size() == 0)
	{
		TRACE_WARNING("bit buffer empty");
		return E_BITBUFFER_EMPTY;
	}
	if(size <= m_head_cache_bits)
	{
		std::string cache = m_head_cache.to_string().erase(0, 8 - m_head_cache_bits);
		data += cache.substr(0, size);
		m_head_cache = std::bitset<8>(cache.erase(0, size));
		m_head_cache_bits -= size;
	}
	else
	{
		uint32_t left = size;
		data += m_head_cache.to_string().erase(0, 8 - m_head_cache_bits);
		left -= m_head_cache_bits;
		m_head_cache_bits = 0;
		m_head_cache.reset();
		for(;left != 0;)
		{
			if(m_buffer.size())
			{
				m_head_cache = m_buffer.front();
				m_buffer.pop_front();
				if(left > 8)
				{
					left -= 8;
					data += m_head_cache.to_string();
					m_head_cache.reset();
				}
				else
				{
					data += m_head_cache.to_string().substr(0, left);
					m_head_cache = std::bitset<8>(m_head_cache.to_string().erase(0, left));
					m_head_cache_bits = 8 - left;
					left = 0;
				}
			}
			else
			{
				if(m_cache_bits >= left)
				{
					std::string cache = m_cache.to_string().erase(0, 8 - m_cache_bits);
					data += cache.substr(0, left);
					m_cache = std::bitset<8>(cache.erase(0, left));
					m_cache_bits -= left;
					left = 0;
				}
				else
				{
					data += m_cache.to_string().erase(0, 8 - m_cache_bits);
					left -= m_cache_bits;
					m_cache_bits = 0;
					m_cache.reset();
					TRACE_WARNING("no enough bits");
					return E_BITBUFFER_NOT_ENOUGH_BITS;
				}
			}
		}
	}
	return E_BITBUFFER_OK;
}

eBitBufferState BitBuffer::read_bits(uint8_t& data, uint32_t size)
{
	if(size > 8)
	{
		TRACE_WARNING("invalid arguments");
		return E_BITBUFFER_INVALID_ARGUMENT;
	}
	std::string value;
	eBitBufferState state = read_bits(value, size);
	if(state == E_BITBUFFER_OK)
	{
		std::bitset<8> tmp(value);
		data = static_cast<uint8_t>(tmp.to_ulong());
	}
	return state;
}

eBitBufferState BitBuffer::read_bits(uint16_t& data, uint32_t size)
{
	if(size > 16)
	{
		TRACE_WARNING("invalid arguments");
		return E_BITBUFFER_INVALID_ARGUMENT;
	}
	std::string value;
	eBitBufferState state = read_bits(value, size);
	if(state == E_BITBUFFER_OK)
	{
		std::bitset<16> tmp(value);
		data = static_cast<uint8_t>(tmp.to_ulong());
	}
	return E_BITBUFFER_OK;
}

eBitBufferState BitBuffer::read_bits(uint32_t& data, uint32_t size)
{
	if(size > 32)
	{
		TRACE_WARNING("invalid arguments");
		return E_BITBUFFER_INVALID_ARGUMENT;
	}
	std::string value;
	eBitBufferState state = read_bits(value, size);
	if(state == E_BITBUFFER_OK)
	{
		std::bitset<32> tmp(value);
		data = static_cast<uint8_t>(tmp.to_ulong());
	}
	return E_BITBUFFER_OK;
}
