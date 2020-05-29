#include "misc.h"

float avg(float const *buffer, size_t buffer_len)
{
	float sum = 0;

	for (size_t i = 0; i < buffer_len; ++i)
	{
		sum += buffer[i];
	}

	return sum / (float) buffer_len;
}
