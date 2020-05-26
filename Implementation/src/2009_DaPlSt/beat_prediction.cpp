#include "2009_DaPlSt/beat_prediction.h"

#include "2009_DaPlSt/constants.h"
#include <cstring>

// see equation (2) in [2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio].
const float ALPHA = 0.9;

// see equation (3) in [2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio].
const float ETA = 5.0;


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

BeatPrediction::BeatPrediction() : past_score(2 * TAU_MAX)
{
	this->current_score = 0;
	this->future_score = new float[TAU_MAX + 1];
	bzero(this->future_score, sizeof(*this->future_score) * (TAU_MAX + 1));

	this->past_weighting = nullptr;
	this->future_weighting = nullptr;
	this->set_tempo(PREFERRED_TEMPO);
}

BeatPrediction::~BeatPrediction()
{
	delete [] this->future_score;
	delete [] this->past_weighting;
	delete [] this->future_weighting;
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
