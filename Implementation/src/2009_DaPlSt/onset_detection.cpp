#include "2009_DaPlSt/onset_detection.h"

#include <cmath>
#include <cstdlib>


OnsetDetector::OnsetDetector(size_t n_bins)
{
	this->n_bins = n_bins;
	this->prev_frames[0] = new Complex<float>[n_bins];
	this->prev_frames[1] = new Complex<float>[n_bins];
}

OnsetDetector::~OnsetDetector()
{
	delete [] this->prev_frames[0];
	delete [] this->prev_frames[1];
}

float OnsetDetector::next_sample(Complex<float> const *stft_frame)
{
	if (stft_frame == nullptr) {
		return NAN;
	}

	float result = 0.0;

	// for each frequency bin
	for (size_t bin = 0; bin < this->n_bins; ++bin) {
		Complex<float> target = Complex<float>(Polar<float>(
			this->prev_frames[0][bin].mag(),
			2 * this->prev_frames[0][bin].phase() - this->prev_frames[1][bin].phase() //
		));
		Complex<float> actual = stft_frame[bin];

		result += (target - actual).mag();
	}

	// update previous frame buffers
	auto tmp = this->prev_frames[1];
	this->prev_frames[1] = this->prev_frames[0];
	this->prev_frames[0] = tmp;
	for (size_t i = 0; i < this->n_bins; ++i) {
		this->prev_frames[0][i] = stft_frame[i];
	}

	return result;
}
