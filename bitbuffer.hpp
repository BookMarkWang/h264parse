#ifndef __BIT_BUFFER_HPP__
#define __BIT_BUFFER_HPP__

#include <deque>
#include <bitset>
#include <cstdint>

typedef enum eBitBufferState
{
	E_BITBUFFER_OK,
	E_BITBUFFER_EMPTY,
	E_BITBUFFER_NOT_ENOUGH_BYTES,
	E_BITBUFFER_NOT_ENOUGH_BITS,
	E_BITBUFFER_INVALID_ARGUMENT,
}eBitBufferState;

class BitBuffer
{
public:
	BitBuffer(std::deque<std::bitset<8>> data = std::deque<std::bitset<8>>());
	void add_byte(uint8_t data);
	void add_bytes(std::deque<uint8_t>& data);
	void add_bit(bool data);
	void add_bits(std::string data);
	bool has_bits();
	bool has_bytes();
	uint32_t get_bits_num();
	eBitBufferState read_bytes(std::deque<uint8_t>& data, uint32_t size);
	eBitBufferState read_bits(std::string& data, uint32_t size);
	eBitBufferState read_bits(uint8_t& data, uint32_t size);
	eBitBufferState read_bits(uint16_t& data, uint32_t size);
	eBitBufferState read_bits(uint32_t& data, uint32_t size);
private:
	std::deque<std::bitset<8>> m_buffer;
	std::bitset<8> m_cache;
	std::bitset<8> m_head_cache;
	uint8_t m_cache_bits;
	uint8_t m_head_cache_bits;
};

#endif //__BIT_BUFFER_HPP__
