#include "2001_ChNaUpVe/beat_this.h"

#include <cassert>
#include <cstring>
#include <Gamma/DFT.h>

using namespace gam;

BeatThis::BeatThis() : input_buffer(ANALYSIS_FRAME_SIZE)
{
	this->hanning_window = new float[PADDED_AF_SIZE];
	bzero(this->hanning_window, sizeof(*this->hanning_window) * PADDED_AF_SIZE);

	for (size_t i = 0; i < HANNING_WINDOW_SIZE; ++i)
	{
		this->hanning_window[i] = 0.5f *
			(1 + cosf(2.0f * M_PI * (float)i / (float)HANNING_WINDOW_SIZE));
	}

	RFFT<float> dft(PADDED_AF_SIZE);
	dft.forward(this->hanning_window);

	this->time = 0;
	this->beat_period = MIN_BP;

}

BeatThis::BeatThis(const BeatThis &that) : input_buffer(that.input_buffer)
{
	this->hanning_window = new float[PADDED_AF_SIZE];
	memcpy(
		this->hanning_window, that.hanning_window,
		sizeof(*this->hanning_window) * PADDED_AF_SIZE
	);

	this->time = that.time;
	this->beat_period = that.beat_period;
}

BeatThis &BeatThis::operator=(const BeatThis &that)
{
	if (this != &that)
	{
		delete [] this->hanning_window;
		this->hanning_window = new float [PADDED_AF_SIZE];
		memcpy(
			this->hanning_window, that.hanning_window,
			sizeof(*this->hanning_window) * PADDED_AF_SIZE
		);

		this->input_buffer = that.input_buffer;
		this->time = that.time;
		this->beat_period = that.beat_period;
	}

	return *this;
}

BeatThis::~BeatThis()
{
	delete [] this->hanning_window;
}

size_t BeatThis::get_beat_period(float *padded_af)
{
	// compute borders of the six frequency ranges
	size_t range_borders[7];

	range_borders[0] = 0;
	range_borders[6] = PADDED_AF_SIZE;

	for (int i = 1; i < 6; ++i)
	{
		float frequency = 200.0f * (float)(1 << (i - 1));
		range_borders[i] = roundf(frequency / HZ_PER_BIN);

		assert(range_borders[i] < PADDED_AF_SIZE);
	}

	// FFT
	RFFT<float> dft(PADDED_AF_SIZE);
	dft.forward(padded_af, false, true);

	// comb filter energies (one for each tempo)
	float energies[N_COMB_FILTERS];
	bzero(energies, sizeof(*energies) * N_COMB_FILTERS);

	// for each frequency band
	for (int k = 0; k < 6; ++k)
	{
		// only copy the frequencies of this band pass filter
		float buffer[PADDED_AF_SIZE];
		bzero(buffer, sizeof(*buffer) * PADDED_AF_SIZE);
		memcpy(
			buffer + range_borders[k], padded_af + range_borders[k],
			sizeof(*buffer) * (range_borders[k + 1] - range_borders[k])
		);

		// IFFT
		dft.inverse(buffer);

		// full-wave rectification
		for (size_t i = 0; i < PADDED_AF_SIZE; ++i)
		{
			if (buffer[i] < 0)
			{
				buffer[i] = -buffer[i];
			}
		}

		// FFT
		dft.forward(buffer);

		// smoothing
		buffer[0] *= this->hanning_window[0];
		buffer[PADDED_AF_SIZE - 1] *= this->hanning_window[PADDED_AF_SIZE - 1];

		for (size_t i = 1; i < PADDED_AF_SIZE - 1; i += 2)
		{
			Complex<float> a = Complex<float>(buffer[i], buffer[i + 1]);
			Complex<float> b = Complex<float>(this->hanning_window[i], this->hanning_window[i + 1]);
			a *= b;
			buffer[i] = a.real();
			buffer[i + 1] = a.imag();
		}

		// IFFT
		dft.inverse(buffer);

		// differentiation
		float last_result = buffer[0];
		size_t i;

		for (i = 1; i < PADDED_AF_SIZE; ++i)
		{
			float result = buffer[i] - buffer[i - 1];
			buffer[i - 1] = last_result;
			last_result = result;
		}

		buffer[i - 1] = last_result;

		// half-wave rectification
		for (size_t i = 0; i < PADDED_AF_SIZE; ++i)
		{
			if (buffer[i] < 0.0)
			{
				buffer[i] = 0.0;
			}
		}

		// comb filters
		for (size_t c = 0; c < N_COMB_FILTERS; ++c)
		{
			size_t bp = MIN_BP + c * BP_STEP;

			for (size_t i = 0; i < PADDED_AF_SIZE; ++i)
			{
				float value =
					buffer[i] +
					(i + bp < PADDED_AF_SIZE ? buffer[i + bp] : 0) +
					(i + 2 * bp < PADDED_AF_SIZE ? buffer[i + 2 * bp] : 0);
				energies[c] += value * value;
			}
		}
	}

	// pick tempo of the maximum energy
	float max = -INFINITY;
	size_t c_max = 0;

	for (size_t c = 0; c < N_COMB_FILTERS; ++c)
	{
		if (energies[c] > max)
		{
			max = energies[c];
			c_max = c;
		}
	}

	// return the found beat period in samples
	return MIN_BP + c_max * BP_STEP;
}

size_t BeatThis::get_time() const
{
	return this->time;
}

size_t BeatThis::get_beat_period() const
{
	return this->beat_period;
}

bool BeatThis::operator()(float sample)
{
	++this->time;

	this->input_buffer.push(sample);

	if (this->time % TEMPO_UPDATE_INTERVAL != 0)
	{
		return false;
	}

	float padded_af[PADDED_AF_SIZE];
	bzero(padded_af, sizeof(*padded_af) * PADDED_AF_SIZE);
	this->input_buffer.get_content(padded_af + HANNING_WINDOW_SIZE);

	this->beat_period = this->get_beat_period(padded_af);

	return true;
}
