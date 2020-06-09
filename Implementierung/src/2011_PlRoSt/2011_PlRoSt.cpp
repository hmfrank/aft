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
	size_t stft_frame_size = sizeof(*this->stft_frame) * get_num_bins();
	size_t matrix_size = sizeof(*this->x_matrix) * MATRIX_WIDTH * MATRIX_HEIGHT;

	this->allocation_ptr = ::operator new(
		stft_frame_size + 2 * matrix_size
	);

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpointer-arith"

	this->stft_frame = static_cast<Complex<float>*>(
		this->allocation_ptr
	);
	this->x_matrix = static_cast<float*>(
		static_cast<void*>(this->stft_frame) + stft_frame_size
	);
	this->y_matrix = static_cast<float*>(
		static_cast<void*>(this->x_matrix) + matrix_size
	);

	#pragma GCC diagnostic pop
}

void _2011_PlRoSt::initialize_stft()
{
	this->stft = new STFT(
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
	this->onset_detection = OnsetDetection(get_num_bins());
	this->af_median = 0;
	this->odf_sample = 0;
	this->time = -1;
	this->current_tau = BETA;
	this->current_x = 0;

	this->allocate_memory();
	this->initialize_stft();

	bzero(
		this->stft_frame,
		sizeof(*this->stft_frame) * this->stft->numBins()
	);
	bzero(this->x_matrix, sizeof(*this->x_matrix) * MATRIX_WIDTH * MATRIX_HEIGHT);
	bzero(this->y_matrix, sizeof(*this->y_matrix) * MATRIX_WIDTH * MATRIX_HEIGHT);
}

_2011_PlRoSt::_2011_PlRoSt(const _2011_PlRoSt &that) :
	onset_detection(0), analysis_frame(ANALYSIS_FRAME_SIZE)
{
	this->onset_detection = that.onset_detection;
	this->af_median = that.af_median;
	this->odf_sample = that.odf_sample;
	this->time = that.time;
	this->current_tau = that.current_tau;
	this->current_x = that.current_x;

	this->allocate_memory();
	this->initialize_stft();

	for (size_t k = 0; k < this->stft->numBins(); ++k)
	{
		this->stft_frame[k] = that.stft_frame[k];
	}
	memcpy(
		this->x_matrix,
		that.x_matrix,
		sizeof(*this->x_matrix) * MATRIX_WIDTH * MATRIX_HEIGHT
	);
	memcpy(
		this->y_matrix,
		that.y_matrix,
		sizeof(*this->y_matrix) * MATRIX_WIDTH * MATRIX_HEIGHT
	);
}

_2011_PlRoSt &_2011_PlRoSt::operator=(const _2011_PlRoSt &that)
{
	this->onset_detection = that.onset_detection;
	this->analysis_frame = that.analysis_frame;
	this->af_median = that.af_median;
	this->odf_sample = that.odf_sample;
	this->time = that.time;
	this->current_tau = that.current_tau;
	this->current_x = that.current_x;

	::operator delete(this->allocation_ptr);
	this->allocate_memory();
	this->initialize_stft();

	for (size_t k = 0; k < this->stft->numBins(); ++k)
	{
		this->stft_frame[k] = that.stft_frame[k];
	}
	memcpy(
		this->x_matrix,
		that.x_matrix,
		sizeof(*this->x_matrix) * MATRIX_WIDTH * MATRIX_HEIGHT
	);
	memcpy(
		this->y_matrix,
		that.y_matrix,
		sizeof(*this->y_matrix) * MATRIX_WIDTH * MATRIX_HEIGHT
	);
}

_2011_PlRoSt::~_2011_PlRoSt()
{
	::operator delete(this->allocation_ptr);
	delete this->stft;
}

float _2011_PlRoSt::get_odf_sample() const
{
	return this->odf_sample;
}

float entropy(const float *buffer, size_t buffer_len)
{
	float total = 0;

	for (size_t i = 0; i < buffer_len; ++i)
	{
		total += buffer[i];
	}

	float sum = 0;

	for (size_t i = 0; i < buffer_len; ++i)
	{
		float p = buffer[i] / total;
		sum += -p * logf(p);
	}

	return sum;
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

float gaussian(float center, float current, float sigma)
{
	return expf(
		(-(center - current) * (center - current)) /
		(2.0f * sigma * sigma)
	);
}

/// Computes the maximum weighted value in the given matrix.
/// In other words: This function copmutes the max of equation (4) in the paper.
float max_weighted_value(const float *matrix, size_t tau, size_t x)
{
	float max = -INFINITY;

	for (size_t y_m = 0; y_m < MATRIX_HEIGHT; ++y_m)
	{
		size_t tau_m = y_m + TAU_MIN;

		for (size_t x_m = 0; x_m < tau_m; ++x_m)
		{
			float tempo_weight = gaussian(tau, tau_m, 3.5f);
			float phase_weight = gaussian(x, x_m, 6.0f);
			float value =
				tempo_weight * phase_weight *
				matrix[y_m * MATRIX_WIDTH + x_m];

			if (value > max)
			{
				max = value;
			}
		}
	}

	return max;
}

/// Computes the median of the given shift register.
float median(const ShiftRegister *sr)
{
	float buffer[sr->get_len()];
	sr->get_content(buffer);

	qsort(buffer, sizeof(*buffer), sr->get_len(), float_compare);

	return buffer[sr->get_len() / 2];
}

/// see equation (6) in the paper
float rayleigh_weight(size_t tau)
{
	float t = (float)tau;
	float b = (float)BETA;

	return t * expf(-t * t / 2.0f / b / b) / b / b;
}

/// Returns the tempo update weight described in equation (10) in the paper.
float tempo_update_weight(size_t tau, size_t tau_new, size_t x, size_t x_new)
{
	return gaussian(tau, tau_new, 4) * gaussian(x, x_new, 10);
}

bool _2011_PlRoSt::operator()(float sample)
{
	if (!(*this->stft)(sample))
	{
		return false;
	}

	++this->time;

	// get STFT frame
	for (size_t k = 0; k < this->stft->numBins(); ++k)
	{
		this->stft_frame[k] = this->stft->bin(k);
	}

	// get next ODF sample
	this->odf_sample = this->onset_detection(this->stft_frame);
	this->analysis_frame.push(this->odf_sample);
	this->af_median = median(&this->analysis_frame);
	// pre-processed ODF sample
	float pp_odf_sample = max(0.0f, this->odf_sample - this->af_median);

	// update X-Matrix
	float updates[MATRIX_HEIGHT];

	for (size_t y = 0; y < MATRIX_HEIGHT; ++y)
	{
		size_t tau = y + TAU_MIN;
		size_t x = this->time % tau;

		updates[y] =
			ALPHA * pp_odf_sample +
			(1 - ALPHA) * max_weighted_value(this->x_matrix, tau, x);
	}

	for (size_t y = 0; y < MATRIX_HEIGHT; ++y)
	{
		size_t tau = y + TAU_MIN;
		size_t x = this->time % tau;

		this->x_matrix[y * MATRIX_WIDTH + x] = updates[y];
	}

	// compute Y-Matrix
	float max = -INFINITY;
	size_t tau_new = 0;
	size_t x_new = 0;

	for (size_t y = 0; y < MATRIX_HEIGHT; ++y)
	{
		size_t tau = y + TAU_MIN;
		float r = rayleigh_weight(tau);
		float h = entropy(this->x_matrix + y * MATRIX_WIDTH, tau);

		for (size_t x = 0; x < tau; ++x)
		{
			float value = this->x_matrix[y * MATRIX_WIDTH + x] * r / h;
			this->y_matrix[y * MATRIX_WIDTH + x] = value;

			if (value > max)
			{
				max = value;
				tau_new = tau;
				x_new = x;
			}
		}
	}

	// update tempo and phase estimates
	float weight = tempo_update_weight(
		this->current_tau, tau_new,
		this->current_x, x_new
	);
	float y_matrix_value = this->y_matrix[this->current_tau * MATRIX_WIDTH + this->current_x];
	float y_matrix_value_new = this->y_matrix[tau_new * MATRIX_WIDTH + x_new];

	if (weight * y_matrix_value_new > y_matrix_value)
	{
		this->current_tau = tau_new;
		this->current_x = x_new;
	}

	return true;
}
