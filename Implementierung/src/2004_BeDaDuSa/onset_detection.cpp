#include "2004_BeDaDuSa/onset_detection.h"

#include <cmath>
#include <cstring>


OnsetDetection::OnsetDetection(size_t n_bins)
{
	this->n_bins = n_bins;
	this->p_frame = new Complex<float>[2 * n_bins];
	this->pp_frame = this->p_frame + n_bins;
}

OnsetDetection::OnsetDetection(const OnsetDetection &that)
{
	this->n_bins = that.n_bins;

	this->p_frame = new Complex<float>[2 * n_bins];
	this->pp_frame = this->p_frame + n_bins;

	for (size_t i = 0; i < n_bins; ++i)
	{
		this->p_frame[i] = that.p_frame[i];
		this->pp_frame[i] = that.pp_frame[i];
	}
}

OnsetDetection &OnsetDetection::operator=(const OnsetDetection &that)
{
	if (this != &that)
	{
		this->n_bins = that.n_bins;

		delete [] this->p_frame;
		this->p_frame = new Complex<float>[2 * n_bins];
		this->pp_frame = this->p_frame + n_bins;

		for (size_t i = 0; i < n_bins; ++i)
		{
			this->p_frame[i] = that.p_frame[i];
			this->pp_frame[i] = that.pp_frame[i];
		}
	}

	return *this;
}

OnsetDetection::~OnsetDetection()
{
	delete [] (
		(size_t)this->p_frame < (size_t)this->pp_frame ?
		this->p_frame : this->pp_frame
	);
}


float OnsetDetection::next_sample(Complex<float> const *stft_frame)
{
	if (stft_frame == nullptr) {
		return NAN;
	}

	float result = 0.0;

	// for each frequency bin
	for (size_t bin = 0; bin < this->n_bins; ++bin)
	{
		Complex<float> target = Complex<float>(Polar<float>(
			this->p_frame[bin].mag(),
			2 * this->p_frame[bin].phase() - this->pp_frame[bin].phase() //
		));
		Complex<float> actual = stft_frame[bin];

		result += (target - actual).mag();
	}

	// update previous frame buffers
	auto tmp = this->pp_frame;
	this->pp_frame = this->p_frame;
	this->p_frame = tmp;
	for (size_t i = 0; i < this->n_bins; ++i) {
		this->p_frame[i] = stft_frame[i];
	}

	return result;
}
