#include "2009_DaPlSt/main.h"

#include "2009_DaPlSt/constants.h"


_2009_DaPlSt::_2009_DaPlSt(float sample_rate) : onset_detection(0)
{
	unsigned int hop_size = (unsigned int)roundf(ODF_SAMPLE_INTERVAL * sample_rate);
	this->stft = STFT(2 * hop_size, hop_size, 0, HANN, COMPLEX);

	this->onset_detection = OnsetDetector(stft.numBins());
	this->tempo_induction = TempoInduction();
	this->beat_prediction = BeatPrediction();

	this->time = 0;
	this->next_beat = 0;
}

size_t _2009_DaPlSt::get_time() const
{
	return this->time;
}

size_t _2009_DaPlSt::get_next_beat_time() const
{
	return this->next_beat;
}

float _2009_DaPlSt::get_tempo() const
{
	return this->tempo_induction.get_tempo();
}

int _2009_DaPlSt::next(float sample)
{
	if (!this->stft(sample))
	{
		return 0;
	}

	++this->time;

	Complex<float> stft_frame[stft.numBins()];

	for (size_t k = 0; k < stft.numBins(); ++k)
	{
		stft_frame[k] = stft.bin(k);
	}

	float odf_sample = this->onset_detection.next_sample(stft_frame);
	int result = 1;

	if (this->tempo_induction.next_sample(odf_sample))
	{
		// new tempo estimate is available
		float tempo = this->tempo_induction.get_tempo();
		this->beat_prediction.set_tempo(tempo);
		++result;
	}

	this->next_beat = this->time + this->beat_prediction.next_prediction(odf_sample);

	return result;
}
