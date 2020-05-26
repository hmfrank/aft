#include "shift_register.h"

#include <cstring>

ShiftRegister::ShiftRegister(size_t len)
{
	this->start = 0;
	this->len = len;
	this->data = new float[len];
	bzero(this->data, sizeof(*this->data) * len);
}

ShiftRegister::~ShiftRegister()
{
	delete[] this->data;
}

void ShiftRegister::get_content(float *buffer) const
{
	if (buffer != nullptr)
	{
		for (size_t i = 0; i < this->len; ++i)
		{
			buffer[i] = this->data[(i + this->start) % this->len];
		}
	}
}

size_t ShiftRegister::get_len() const
{
	return this->len;
}

float ShiftRegister::push(float value)
{
	if (this->len == 0)
	{
		return value;
	}

	float removed_item = this->data[this->start];

	this->data[this->start] = value;
	this->start = (this->start + 1) % this->len;

	return removed_item;
}

float ShiftRegister::operator[](size_t index)
{
	return this->data[(this->start + index) % this->len];
}

void ShiftRegister::push_range(const float *buffer, size_t buffer_len)
{
	if (buffer_len >= this->len)
	{
		// copy the last past of `buffer` to the entire `this->data` array
		//
		//         +-----+---------+-----+-----+-----+
		// buffer: |     |   ...   |     |     |     |
		//         +-----+---------+-----+-----+-----+
		//                            |     |     |   memcpy
		//                            V     V     V
		//                         +-----+-----+-----+
		// this->data:             |     |     |     |
		//                         +-----+-----+-----+

		size_t n_excess = buffer_len - this->len;
		memcpy(this->data, buffer + n_excess, sizeof(*buffer) * this->len);
		this->start = 0;
	}
	else
	{
		size_t new_start = (this->start + buffer_len) % this->len;

		if (this->start + buffer_len <= this->len)
		{
			// copy entire `buffer` to `this->start`
			//
		    //                         +-----+-----+-----+
			// buffer:                 |     |     |     |
			//                         +-----+-----+-----+
			//                            |     |     |   memcpy
			//                            V     V     V
			//             +-----+-----+-----+-----+-----+
			// this->data: |     |     |     |     |     |
			//             +-----+-----+-----+-----+-----+

			memcpy(this->data + this->start, buffer, sizeof(*buffer) * buffer_len);
		}
		else
		{
			// copy first part of `buffer` to the end of `this->data` and the last part to the beginning
			//
			//                         +-----+-----+-----+-----+-----+
			// buffer:                 |     |     |     |     |     |
			//                         +-----+-----+-----+-----+-----+
			//                            |     |     |     |     |
			//                /-----------|-----|-----|-----/     |   2 x memcpy
			//                |     /-----|-----|-----|-----------/
			//                |     |     |     |     |
			//                V     V     V     V     V
			//             +-----+-----+-----+-----+-----+
			// this->data: |     |     |     |     |     |
			//             +-----+-----+-----+-----+-----+

			size_t n_part1 = this->len - this->start;
			size_t n_part2 = buffer_len - n_part1;

			memcpy(this->data + this->start, buffer, sizeof(*buffer) * n_part1);
			memcpy(this->data, buffer + n_part1, sizeof(*buffer) * n_part2);
		}

		this->start = new_start;
	}
}
