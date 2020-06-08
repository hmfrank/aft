#include "misc.h"

#include <cmath>

float avg(float const *buffer, size_t buffer_len)
{
	float sum = 0;

	for (size_t i = 0; i < buffer_len; ++i)
	{
		sum += buffer[i];
	}

	return sum / (float) buffer_len;
}

float max(float const *buffer, size_t buffer_len)
{
	if (buffer_len == 0)
	{
		return NAN;
	}

	float max = buffer[0];

	for (size_t i = 0; i < buffer_len; ++i)
	{
		if (buffer[i] > max)
		{
			max = buffer[i];
		}
	}

	return max;
}

float min(float const *buffer, size_t buffer_len)
{
	if (buffer_len == 0)
	{
		return NAN;
	}

	float min = buffer[0];

	for (size_t i = 0; i < buffer_len; ++i)
	{
		if (buffer[i] < min)
		{
			min = buffer[i];
		}
	}

	return min;
}
