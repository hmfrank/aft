#include "shift_register.h"

ShiftRegister::ShiftRegister(size_t len)
{
	this->start = 0;
	this->len = len;
	this->data = new float [len];
}

ShiftRegister::~ShiftRegister()
{
	delete [] this->data;
}

void ShiftRegister::get_content(float *buffer) const
{
	if (buffer != nullptr) {
		for (size_t i = 0; i < this->len; ++i) {
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
	if (this->len == 0) {
		return value;
	}

	float removed_item = this->data[this->start];

	this->data[this->start] = value;
	this->start = (this->start + 1) % this->len;

	return removed_item;
}
