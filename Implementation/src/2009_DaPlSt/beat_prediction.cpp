#include "2009_DaPlSt/beat_prediction.h"

#include "2009_DaPlSt/constants.h"
#include <cstring>

// TODO: only allocate one block of memory for all 3 arrays


// see equation (2) in [2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio].
const float ALPHA = 0.9;

// see equation (3) in [2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio].
const float ETA = 5.0;


BeatPrediction::BeatPrediction() : past_score(0)
{
	// `set_tempo()` initializes `beat_period`, `past_weighting`, and
	// `future_weighting`.
	this->past_weighting = nullptr;
	this->future_weighting = nullptr;
	this->set_tempo(PREFERRED_TEMPO);

	this->past_score = ShiftRegister(2 * TAU_MAX);
	this->current_score = 0;
	this->future_score = new float[TAU_MAX + 1];
	bzero(this->future_score, sizeof(*this->future_score) * (TAU_MAX + 1));
}

BeatPrediction::BeatPrediction(const BeatPrediction &that) : past_score(0)
{
	this->beat_period = that.beat_period;

	size_t size = 2 * this->beat_period + 1;
	this->past_weighting = new float[size];
	memcpy(this->past_weighting, that.past_weighting, sizeof(*this->past_weighting) * size);

	size = this->beat_period + 1;
	this->future_weighting = new float[size];
	memcpy(this->future_weighting, that.future_weighting, sizeof(*this->future_weighting) * size);

	this->past_score = that.past_score; // should call copy assignment operator
	this->current_score = that.current_score;

	size = TAU_MAX + 1;
	this->future_score = new float[size];
	memcpy(this->future_score, that.future_score, sizeof(*this->future_score) * size);
}

BeatPrediction &BeatPrediction::operator=(const BeatPrediction &that)
{
	if (this != &that)
	{
		this->beat_period = that.beat_period;

		delete [] this->past_weighting;
		size_t size = 2 * this->beat_period + 1;
		this->past_weighting = new float[size];
		memcpy(this->past_weighting, that.past_weighting, sizeof(*this->past_weighting) * size);

		delete [] this->future_weighting;
		size = this->beat_period + 1;
		this->future_weighting = new float[size];
		memcpy(this->future_weighting, that.future_weighting, sizeof(*this->future_weighting) * size);

		this->past_score = that.past_score; // should call copy assignment operator
		this->current_score = that.current_score;

		delete [] this->future_score;
		size = TAU_MAX + 1;
		this->future_score = new float[size];
		memcpy(this->future_score, that.future_score, sizeof(*this->future_score) * size);
	}

	return *this;
}

BeatPrediction::~BeatPrediction()
{
	delete [] this->past_weighting;
	delete [] this->future_weighting;
	delete [] this->future_score;
}


float BeatPrediction::score_function(ssize_t index)
{
	if (index < 0)
	{
		return this->past_score[-index - 1];
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

void BeatPrediction::set_tempo(float tempo)
{
	this->beat_period = roundf(60.0f / ODF_SAMPLE_INTERVAL / tempo);

	delete [] this->past_weighting;
	delete [] this->future_weighting;

	this->past_weighting = new float[2 * this->beat_period + 1];

	for (size_t v = 0; v <= 2 * this->beat_period; ++v)
	{
		// note that this would be W_1(-v) in the paper but here we're using positive offsets even when referring to the past
		this->past_weighting[v] = expf(-powf(ETA * logf((float)v / (float)this->beat_period), 2.0f) / 2.0f);
	}

	this->future_weighting = new float[this->beat_period + 1];

	for (size_t v = 0; v <= this->beat_period; ++v)
	{
		this->future_weighting[v] = expf(-powf((float)v - (float)this->beat_period / 2.0f, 2.0f) / 2.0f / powf(this->beat_period / 2.0f, 2.0f));
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

size_t BeatPrediction::next_prediction(float odf_sample)
{
	this->past_score.push(this->current_score);
	this->current_score = ((1 - ALPHA) * odf_sample + ALPHA * this->eq2rhs(0));

	float max = 0;
	size_t argmax = 0;
	for (size_t m = 1; m <= this->beat_period; ++m)
	{
		float value = this->eq2rhs(m);
		this->future_score[m] = value;

		if (value >= max)
		{
			max = value;
			argmax = m;
		}
	}

	return argmax;
}
