#include "2009_DaPlSt/beat_prediction.h"

#include "2009_DaPlSt/constants.h"
#include <cstring>

const size_t PAST_WGHT_OFFSET = 0;
const size_t FUTURE_WGHT_OFFSET = PAST_WGHT_OFFSET + 2 * TAU_MAX + 1;
const size_t FUTURE_SCORE_OFFSET = FUTURE_WGHT_OFFSET + TAU_MAX + 1;
const size_t ALLOCATION_SIZE = FUTURE_SCORE_OFFSET + TAU_MAX + 1;


// see equation (2) in [2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio].
const float ALPHA = 0.9;

// see equation (3) in [2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio].
const float ETA = 5.0;


BeatPrediction::BeatPrediction() : past_score(2 * TAU_MAX)
{
	this->allocation_ptr = new float[ALLOCATION_SIZE];

	this->past_weighting = this->allocation_ptr + PAST_WGHT_OFFSET;
	this->future_weighting = this->allocation_ptr + FUTURE_WGHT_OFFSET;

	// initialize score function with zeros
	this->current_score = 0;
	this->future_score = this->allocation_ptr + FUTURE_SCORE_OFFSET;
	bzero(this->future_score, sizeof(*this->future_score) * (TAU_MAX + 1));

	// `set_tempo()` initializes `beat_period`, `past_weighting`, and
	// `future_weighting`.
	this->set_tempo(PREFERRED_TEMPO);
}

BeatPrediction::BeatPrediction(const BeatPrediction &that) : past_score(0)
{
	this->allocation_ptr = new float[ALLOCATION_SIZE];

	this->past_weighting = this->allocation_ptr + PAST_WGHT_OFFSET;
	size_t size = 2 * this->beat_period + 1;
	memcpy(this->past_weighting, that.past_weighting, sizeof(*this->past_weighting) * size);

	this->future_weighting = this->allocation_ptr + FUTURE_WGHT_OFFSET;
	size = this->beat_period + 1;
	memcpy(this->future_weighting, that.future_weighting, sizeof(*this->future_weighting) * size);

	this->past_score = that.past_score; // should call copy assignment operator
	this->current_score = that.current_score;
	this->future_score = this->allocation_ptr + FUTURE_SCORE_OFFSET;
	size = TAU_MAX + 1;
	memcpy(this->future_score, that.future_score, sizeof(*this->future_score) * size);

	this->beat_period = that.beat_period;
}

BeatPrediction &BeatPrediction::operator=(const BeatPrediction &that)
{
	if (this != &that)
	{
		delete [] this->allocation_ptr;
		this->allocation_ptr = new float[ALLOCATION_SIZE];

		this->past_weighting = this->allocation_ptr + PAST_WGHT_OFFSET;
		size_t size = 2 * this->beat_period + 1;
		memcpy(this->past_weighting, that.past_weighting, sizeof(*this->past_weighting) * size);

		this->future_weighting = this->allocation_ptr + FUTURE_WGHT_OFFSET;
		size = this->beat_period + 1;
		memcpy(this->future_weighting, that.future_weighting, sizeof(*this->future_weighting) * size);

		this->past_score = that.past_score; // should call copy assignment operator
		this->current_score = that.current_score;
		this->future_score = this->allocation_ptr + FUTURE_SCORE_OFFSET;
		size = TAU_MAX + 1;
		memcpy(this->future_score, that.future_score, sizeof(*this->future_score) * size);

		this->beat_period = that.beat_period;
	}

	return *this;
}

BeatPrediction::~BeatPrediction()
{
	delete [] this->allocation_ptr;
}


float BeatPrediction::score_function(ssize_t index)
{
	if (index < 0)
	{
		return this->past_score[this->past_score.get_len() + index];
	}
	else if (index == 0)
	{
		return this->current_score;
	}
	else
	{
		return this->future_score[index - 1];
	}
}

float BeatPrediction::eq2rhs(ssize_t m)
{
	float max = 0;

	for (ssize_t v = -2 * (ssize_t)this->beat_period; v <= -(ssize_t)this->beat_period / 2; ++v)
	{
		float value = this->past_weighting[-v] * this->score_function(m + v);

		if (value > max)
		{
			max = value;
		}
	}

	return max;
}

float BeatPrediction::get_beat_period() const
{
	return this->beat_period;
}

float BeatPrediction::get_current_score() const
{
	return this->current_score;
}

const float *BeatPrediction::get_future_score() const
{
	return this->future_score;
}

void BeatPrediction::set_tempo(float tempo)
{
	this->beat_period = roundf(60.0f / ODF_SAMPLE_INTERVAL / tempo);

	for (size_t v = 0; v <= 2 * this->beat_period; ++v)
	{
		// note that this would be W_1(-v) in the paper but here we're using positive offsets even when referring to the past
		float value = expf(-powf(ETA * logf((float)v / (float)this->beat_period), 2.0f) / 2.0f);
		this->past_weighting[v] = value;
	}

	for (size_t v = 0; v <= this->beat_period; ++v)
	{
		this->future_weighting[v] = expf(-powf((float)v - (float)this->beat_period / 2.0f, 2.0f) / 2.0f / powf(this->beat_period / 2.0f, 2.0f));
	}
}

size_t BeatPrediction::next_prediction(float odf_sample)
{
	this->past_score.push(this->current_score);
	this->current_score = ((1 - ALPHA) * odf_sample + ALPHA * this->eq2rhs(0));

	float max = 0;
	size_t argmax = 0;
	for (size_t m = 1; m <= this->beat_period; ++m)
	{
		float value = this->eq2rhs(m);
		this->future_score[m - 1] = value;

		if (value >= max)
		{
			max = value;
			argmax = m;
		}
	}

	return argmax;
}
