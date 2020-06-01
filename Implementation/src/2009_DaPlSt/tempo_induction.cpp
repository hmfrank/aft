#include "2009_DaPlSt/tempo_induction.h"

#include <cmath>
#include <cstring>
#include "misc.h"


const size_t TTM_SIZE = TAU_MAX - TAU_MIN + 1;

// Tempo transition matrix from equation (8) in [2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio].
// This matrix is a square of side length `TTM_SIZE` and is allocated an initialized once, namely the first time the
// constructor of `TempoInduction` is called.
float *TEMPO_TRANSITION_MATRIX = nullptr;

// number of floats to allocate for one instance of the class
const size_t ALLOCATION_SIZE = TTM_SIZE + 2 * ANALYSIS_FRAME_SIZE;
const size_t SP_OFFSET = 0;
const size_t MAF_OFFSET = TTM_SIZE;
const size_t ACF_OFFSET = MAF_OFFSET + ANALYSIS_FRAME_SIZE;


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

	this->allocation_ptr = new float[ALLOCATION_SIZE];

	this->state_probabilities = this->allocation_ptr + SP_OFFSET;
	for (size_t i = 0; i < TTM_SIZE; ++i)
	{
		this->state_probabilities[i] = 1.0f;
	}

	this->modified_analysis_frame = this->allocation_ptr + MAF_OFFSET;
	this->acf = this->allocation_ptr + ACF_OFFSET;
	bzero(
		this->modified_analysis_frame,
		sizeof(*this->acf) * 2 * ANALYSIS_FRAME_SIZE
	);
}

TempoInduction::TempoInduction(const TempoInduction &that) : input_buffer(that.input_buffer)
{
	this->current_tempo = that.current_tempo;
	this->n_new_samples = that.n_new_samples;

	this->allocation_ptr = new float[ALLOCATION_SIZE];

	this->state_probabilities = this->allocation_ptr + SP_OFFSET;
	memcpy(
		this->state_probabilities,
		that.state_probabilities,
		sizeof(*this->state_probabilities) * TTM_SIZE
	);

	this->modified_analysis_frame = this->allocation_ptr + MAF_OFFSET;
	this->acf = this->allocation_ptr + ACF_OFFSET;
	memcpy(
		this->modified_analysis_frame,
		that.modified_analysis_frame,
		sizeof(*this->acf) * 2 * ANALYSIS_FRAME_SIZE
	);
}

TempoInduction &TempoInduction::operator=(const TempoInduction &that)
{
	if (this != &that)
	{
		this->current_tempo = that.current_tempo;
		this->n_new_samples = that.n_new_samples;
		this->input_buffer = that.input_buffer; // should call the copy assignment operator

		delete [] this->allocation_ptr;
		this->allocation_ptr = new float[ALLOCATION_SIZE];

		this->state_probabilities = this->allocation_ptr + SP_OFFSET;
		memcpy(
			this->state_probabilities,
			that.state_probabilities,
			sizeof(*this->state_probabilities) * TTM_SIZE
		);

		this->modified_analysis_frame = this->allocation_ptr + MAF_OFFSET;
		this->acf = this->allocation_ptr + ACF_OFFSET;
		memcpy(
			this->modified_analysis_frame,
			that.modified_analysis_frame,
			sizeof(*this->acf) * 2 * ANALYSIS_FRAME_SIZE
		);
	}

	return *this;
}

TempoInduction::~TempoInduction()
{
	delete [] this->allocation_ptr;
}


const float *TempoInduction::get_acf() const
{
	return this->acf;
}

const float *TempoInduction::get_modified_analysis_frame() const
{
	return this->modified_analysis_frame;
}

size_t TempoInduction::get_n_new_samples() const
{
	return this->n_new_samples;
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
	for (size_t i = 0; i < ANALYSIS_FRAME_SIZE; ++i)
	{
		this->modified_analysis_frame[i] = analysis_frame[i] - avg(analysis_frame + i - 8, 17);
		if (this->modified_analysis_frame[i] < 0) modified_analysis_frame[i] = 0;
	}

	// compute auto correlation function
	autocorrelation(modified_analysis_frame, ANALYSIS_FRAME_SIZE, this->acf);

	// find best tau
	float max_sum = -INFINITY;
	size_t best_tau = TAU_MIN;

	for (size_t tau = TAU_MIN; tau <= TAU_MAX; ++tau) // tau = spacing of comb filter elements in ODF samples
	{
		float weight = comb_filter_weight(tau);
		float sum = 0;

		for (ssize_t p = 1; p <= 4; ++p)
		{
			for (ssize_t v = 1 - p; v <= p - 1; ++v)
			{
				ssize_t lag = tau * p + v;

				sum += this->acf[lag] * weight / (float) (2 * p - 1);
			}
		}

		if (sum > max_sum)
		{
			max_sum = sum;
			best_tau = tau;
		}
	}

	// We don't actually need this much memory for the comb filter output
	// but it's easier this way.
	float weight = comb_filter_weight(best_tau);
	float comb_filter_output[ANALYSIS_FRAME_SIZE];
	bzero(comb_filter_output, sizeof(*comb_filter_output) * ANALYSIS_FRAME_SIZE);

	for (ssize_t p = 1; p <= 4; ++p)
	{
		for (ssize_t v = 1 - p; v <= p - 1; ++v)
		{
			ssize_t lag = best_tau * p + v;

			comb_filter_output[lag] = this->acf[lag] * weight / (float) (2 * p - 1);
		}
	}
	this->current_tempo = 60.0f / ODF_SAMPLE_INTERVAL / (float)best_tau;
	return true;

	// update stored state probabilities
	float sp_total = 0;
	float prev_sp[TTM_SIZE];
	memcpy(prev_sp, this->state_probabilities, sizeof(float) * TTM_SIZE);

	for (size_t i = 0; i < TTM_SIZE; ++i)
	{
		float sum = 0;

		for (size_t j = 0; j < TTM_SIZE; ++j)
		{
			sum += TEMPO_TRANSITION_MATRIX[i * TTM_SIZE + j] * prev_sp[i];
		}

		this->state_probabilities[i] = sum * comb_filter_output[i + TAU_MIN];
		sp_total += this->state_probabilities[i];
	}

	// normalize and find maximum
	float max = -INFINITY;
	size_t index_max = 0;

	for (size_t i = 0; i < TTM_SIZE; ++i)
	{
		this->state_probabilities[i] /= sp_total;

		if (this->state_probabilities[i] >= max)
		{
			max = this->state_probabilities[i];
			index_max = i;
		}
	}

	this->current_tempo = 60.0f / ODF_SAMPLE_INTERVAL / (float)(index_max + TAU_MIN);

	return true;
}

float TempoInduction::comb_filter_weight(size_t tau)
{
	return
		(float) tau / (float) (BETA * BETA) *
		expf(-(float) (tau * tau) / (2 * (float) (BETA * BETA)));
}
