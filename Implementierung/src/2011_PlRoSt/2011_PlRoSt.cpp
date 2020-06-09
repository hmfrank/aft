#include "2011_PlRoSt/2011_PlRoSt.h"

#include "2011_PlRoSt/constants.h"
#include <cstdlib>
#include "misc.h"


size_t get_num_bins()
{
	STFT stft(
		STFT_WINDOW_SIZE,
		STFT_HOP_SIZE,
		0,
		HANN,
		COMPLEX
	);

	return stft.numBins();
}

void _2011_PlRoSt::allocate_memory()
{
	size_t stft_size = sizeof(*this->stft);
	size_t stft_frame_size = sizeof(*this->stft_frame) * get_num_bins();
	size_t matrix_size = sizeof(*this->matrix) * MATRIX_WIDTH * MATRIX_HEIGHT;

	this->allocation_ptr = ::operator new(
		stft_size + stft_frame_size + matrix_size
	);

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpointer-arith"

	this->stft = static_cast<STFT*>(this->allocation_ptr);
	this->stft_frame = static_cast<Complex<float>*>(
		this->allocation_ptr + stft_size
	);
	this->matrix = static_cast<float*>(
		static_cast<void*>(this->stft_frame) + stft_frame_size
	);

	#pragma GCC diagnostic pop
}

void _2011_PlRoSt::initialize_stft()
{
	*this->stft = STFT(
		STFT_WINDOW_SIZE,
		STFT_HOP_SIZE,
		0,
		HANN,
		COMPLEX
	);
}

_2011_PlRoSt::_2011_PlRoSt() :
	onset_detection(0), analysis_frame(ANALYSIS_FRAME_SIZE)
{
	this->onset_detection = OnsetDetection(this->stft->numBins());
	this->af_median = 0;
	this->time = -1;

	this->allocate_memory();
	this->initialize_stft();

	bzero(
		this->stft_frame,
		sizeof(*this->stft_frame) * this->stft->numBins()
	);
	bzero(this->matrix, sizeof(*this->matrix) * MATRIX_WIDTH * MATRIX_HEIGHT);
}

_2011_PlRoSt::_2011_PlRoSt(const _2011_PlRoSt &that) :
	onset_detection(0), analysis_frame(ANALYSIS_FRAME_SIZE)
{
	this->onset_detection = that.onset_detection;
	this->af_median = that.af_median;
	this->time = that.time;

	this->allocate_memory();
	this->initialize_stft();

	for (size_t k = 0; k < this->stft->numBins(); ++k)
	{
		this->stft_frame[k] = that.stft_frame[k];
	}
	memcpy(
		this->matrix,
		that.matrix,
		sizeof(*this->matrix) * MATRIX_WIDTH * MATRIX_HEIGHT
	);
}

_2011_PlRoSt &_2011_PlRoSt::operator=(const _2011_PlRoSt &that)
{
	this->onset_detection = that.onset_detection;
	this->analysis_frame = that.analysis_frame;
	this->af_median = that.af_median;
	this->time = that.time;

	::operator delete(this->allocation_ptr);
	this->allocate_memory();
	this->initialize_stft();

	for (size_t k = 0; k < this->stft->numBins(); ++k)
	{
		this->stft_frame[k] = that.stft_frame[k];
	}
	memcpy(
		this->matrix,
		that.matrix,
		sizeof(*this->matrix) * MATRIX_WIDTH * MATRIX_HEIGHT
	);
}

_2011_PlRoSt::~_2011_PlRoSt()
{
	::operator delete(this->allocation_ptr);
}

size_t _2011_PlRoSt::get_n_bins() const
{
	return this->stft->numBins();
}

const Complex<float> *_2011_PlRoSt::get_stft_frame() const
{
	return this->stft_frame;
}

/// float comparison function for `qsort()`
int float_compare(const void *a, const void *b)
{
	const float *x = static_cast<const float*>(a);
	const float *y = static_cast<const float*>(b);

	if (*x < *y)
	{
		return -1;
	}
	else if (*x > *y)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/// Computes the median of the given shift register.
float get_median(const ShiftRegister *sr)
{
	if (sr == nullptr || sr->get_len() == 0)
	{
		return NAN;
	}

	float buffer[sr->get_len()];
	sr->get_content(buffer);

	qsort(buffer, sizeof(*buffer), sr->get_len(), float_compare);

	return buffer[sr->get_len() / 2];
}

int _2011_PlRoSt::operator()(float sample)
{
	if (!(*this->stft)(sample))
	{
		return 0;
	}

	++this->time;

	// get STFT frame
	for (size_t k = 0; k < this->stft->numBins(); ++k)
	{
		this->stft_frame[k] = this->stft->bin(k);
	}

	// get next ODF sample
	float odf_sample = this->onset_detection(this->stft_frame);
	this->analysis_frame.push(odf_sample);
	this->af_median = get_median(&this->analysis_frame);
	// pre-processed ODF sample
	float pp_odf_sample = max(0.0f, odf_sample - this->af_median);

	// update matrix
	float updates[MATRIX_HEIGHT];

	for (size_t y = 0; y < MATRIX_HEIGHT; ++y)
	{
		size_t tau = y + TAU_MIN;
		size_t x = this->time % tau;

		float max = -INFINITY;

		for (size_t y_m = 0; y < MATRIX_HEIGHT; ++y)
		{
			size_t tau_m = y_m + TAU_MIN;

			for (size_t x_m = 0; x_m < tau_m; ++x_m)
			{
				float tempo_weight = expf(
					(float)(-(tau - tau_m) * (tau - tau_m)) /
					(2.0f * 3.5f * 3.5f)
				);
				float phase_weight = expf(
					(float)(-(x - x_m) * (x - x_m)) /
					(2.0f * 6.0f * 6.0f)
				);
				float value =
					tempo_weight * phase_weight *
					matrix[y_m * MATRIX_WIDTH + x_m];

				if (value > max)
				{
					max = value;
				}
			}
		}

		updates[y] = ALPHA * pp_odf_sample + (1 - ALPHA) * max;
	}

	for (size_t y = 0; y < MATRIX_HEIGHT; ++y)
	{
		size_t tau = y + TAU_MIN;
		size_t x = this->time % tau;

		this->matrix[y * MATRIX_WIDTH + x] = updates[y];
	}

	// TODO: continue here

	return 1;
}
