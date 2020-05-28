#include "2009_DaPlSt/tempo_induction.h"

#include "2009_DaPlSt/constants.h"
#include <cmath>
#include <cstring>


// analysis frame size in seconds
const float ANALYSIS_FRAME_SIZE_S = 5.94432;
// analysis frame size in ODF-samples
const size_t ANALYSIS_FRAME_SIZE = roundf(ANALYSIS_FRAME_SIZE_S / ODF_SAMPLE_INTERVAL); // = 512

// after this many seconds a new tempo induction starts with the new analysis frame
const float ANALYSIS_FRAME_SHIFT_S = 1.48608;
// analysis frame shift in ODF-samples
const size_t ANALYSIS_FRAME_SHIFT = roundf(ANALYSIS_FRAME_SHIFT_S / ODF_SAMPLE_INTERVAL); // = 128

// inter-beat-interval of the preferred tempo in ODF samples
// see equation (8) in [2007 Davies, Plumbey - Context-Dependent Beat Tracking of Musical Audio]
const size_t BETA = roundf(60.0f / ODF_SAMPLE_INTERVAL / PREFERRED_TEMPO);


const size_t TTM_SIZE = TAU_MAX - TAU_MIN + 1;
// Tempo transition matrix from equation (8) in [2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio].
// This matrix is a square of side length `TTM_SIZE` and is allocated an initialized once, namely the first time the
// constructor of `TempoInduction` is called.
float *TEMPO_TRANSITION_MATRIX = nullptr;


float *new_ttm()
{
	auto *result = new float [TTM_SIZE * TTM_SIZE];
	float sigma = (MAX_TEMPO - MIN_TEMPO) / 8.0f;

	for (size_t i = 0; i < TTM_SIZE; ++i)
	{
		for (size_t j = 0; j < TTM_SIZE; ++j)
		{
			float t_i = 60.0f / ODF_SAMPLE_INTERVAL / (float)(i + TAU_MIN);
			float t_j = 60.0f / ODF_SAMPLE_INTERVAL / (float)(i + TAU_MAX);

			result[i * TTM_SIZE + j] =
				expf(-(t_i - t_j) * (t_i - t_j) / 2.0f / sigma / sigma) /
				sigma / sqrtf(2.0f * M_PI);
		}
	}

	return result;
}

TempoInduction::TempoInduction() : input_buffer(ANALYSIS_FRAME_SIZE)
{
	if (TEMPO_TRANSITION_MATRIX == nullptr)
	{
		TEMPO_TRANSITION_MATRIX = new_ttm();
	}

	this->current_tempo = PREFERRED_TEMPO;
	this->n_new_samples = 0;
	this->state_probabilities = new float [TTM_SIZE];

	for (size_t i = 0; i < TTM_SIZE; ++i)
	{
		this->state_probabilities[i] = 1.0f;
	}
}

TempoInduction::TempoInduction(const TempoInduction &that) : input_buffer(that.input_buffer)
{
	this->current_tempo = that.current_tempo;
	this->n_new_samples = that.n_new_samples;

	this->state_probabilities = new float[TTM_SIZE];
	memcpy(this->state_probabilities, that.state_probabilities, sizeof(*this->state_probabilities) * TTM_SIZE);
}

TempoInduction &TempoInduction::operator=(const TempoInduction &that)
{
	if (this != &that)
	{
		this->current_tempo = that.current_tempo;
		this->n_new_samples = that.n_new_samples;
		this->input_buffer = that.input_buffer; // should call the copy assignment operator

		delete [] this->state_probabilities;
		this->state_probabilities = new float[TTM_SIZE];
		memcpy(this->state_probabilities, that.state_probabilities, sizeof(*this->state_probabilities) * TTM_SIZE);
	}

	return *this;
}

TempoInduction::~TempoInduction()
{
	delete [] this->state_probabilities;
}


float TempoInduction::get_tempo() const
{
	return this->current_tempo;
}

void autocorrelation(float const *input, size_t input_len, float *output)
{
	for (size_t lag = 0; lag < input_len; ++lag)
	{
		float sum = 0;

		for (size_t i = lag; i < input_len; ++i)
		{
			sum += input[i] * input[i - lag];
		}

		output[lag] = sum / (float) (input_len - lag);
	}
}

float avg(float const *buffer, size_t buffer_len)
{
	float sum = 0;

	for (size_t i = 0; i < buffer_len; ++i)
	{
		sum += buffer[i];
	}

	return sum / (float) buffer_len;
}

bool TempoInduction::next_sample(float odf_sample)
{
	this->input_buffer.push(odf_sample);

	// we only perform a new analysis evey ANALYSIS_FRAME_SHIFT samples
	if (++this->n_new_samples < ANALYSIS_FRAME_SHIFT)
	{
		return false;
	}

	this->n_new_samples = 0;

	// extract analysis frame with 8 zeros padding on either side
	float buffer[ANALYSIS_FRAME_SIZE + 16];
	float *analysis_frame = buffer + 8;
	this->input_buffer.get_content(analysis_frame);

	// subtract moving mean and set negative values to 0
	float modified_analysis_frame[ANALYSIS_FRAME_SIZE];
	for (size_t i = 0; i < ANALYSIS_FRAME_SIZE; ++i)
	{
		modified_analysis_frame[i] = analysis_frame[i] - avg(analysis_frame + i - 8, 17);
		if (modified_analysis_frame[i] < 0) modified_analysis_frame[i] = 0;
	}

	// compute auto correlation function
	float acf[ANALYSIS_FRAME_SIZE];
	autocorrelation(modified_analysis_frame, ANALYSIS_FRAME_SIZE, acf);

	// comb filter stuff
	float max_sum = -INFINITY;
	// Since we only need a portion of the entire comb filter-bank output, we store an excerpt of the entire
	// output in `comb_filter_output`.
	// This excerpt ranges from index `TAU_MIN` to `TAU_MAX` (inclusive).
	// So index 0 of `comb_filter_output` would be index `TAU_MIN` of the actual entire comb filter-bank output.
	float comb_filter_output[TTM_SIZE];

	for (size_t tau = TAU_MIN; tau <= TAU_MAX; ++tau) // tau = spacing of comb filter elements in ODF samples
	{
		float weight =
				(float) tau / (float) (BETA * BETA) *
				expf(-(float) (tau * tau) / (2 * (float) (BETA * BETA)));
		float sum = 0;

		for (size_t p = 1; p <= 4; ++p)
		{
			for (size_t v = 1 - p; v <= p - 1; ++v)
			{
				size_t lag = tau * p + v;

				sum += acf[lag] * weight / (float) (2 * p - 1);
			}
		}

		if (sum > max_sum)
		{
			max_sum = sum;

			bzero(comb_filter_output, sizeof(*comb_filter_output) * TTM_SIZE);
			for (size_t p = 1; p <= 4; ++p)
			{
				for (size_t v = 1 - p; v <= p - 1; ++v)
				{
					size_t lag = tau * p + v;
					size_t i = lag - TAU_MIN;

					comb_filter_output[i] = acf[lag] * weight / (float) (2 * p - 1);
				}
			}
		}
	}

	// update stored state probabilities
	float prev_sp[TTM_SIZE];
	memcpy(prev_sp, this->state_probabilities, sizeof(float) * TTM_SIZE);

	for (size_t i = 0; i < TTM_SIZE; ++i)
	{
		float sum = 0;

		for (size_t j = 0; j < TTM_SIZE; ++j)
		{
			sum += TEMPO_TRANSITION_MATRIX[i * TTM_SIZE + j] * prev_sp[i];
		}

		this->state_probabilities[i] = sum;
	}

	float sum = 0;
	for (size_t i = 0; i < TTM_SIZE; ++i)
	{
		sum += this->state_probabilities[i] *= comb_filter_output[i];
	}

	// normalize and find maximum
	float max = -INFINITY;
	size_t index_max = 0;

	for (size_t i = 0; i < TTM_SIZE; ++i)
	{
		this->state_probabilities[i] /= sum;

		if (this->state_probabilities[i] >= max)
		{
			max = this->state_probabilities[i];
			index_max = i;
		}
	}

	this->current_tempo = 60.0f / ODF_SAMPLE_INTERVAL / (float)(index_max + TAU_MIN);

	return true;
}
