#include <2011_PlRoSt/constants.h>
#include "2011_PlRoSt/2011_PlRoSt.h"

#include "2011_PlRoSt/constants.h"


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
	size_t size =
		sizeof(*this->stft) +
		sizeof(*this->stft_frame) * get_num_bins();
	this->allocation_ptr = ::operator new(size);

	this->stft = static_cast<STFT*>(this->allocation_ptr);

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpointer-arith"

	this->stft_frame = static_cast<Complex<float>*>(
		this->allocation_ptr + sizeof(*this->stft)
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

_2011_PlRoSt::_2011_PlRoSt(float sample_rate) : onset_detection(0)
{
	this->allocate_memory();
	this->initialize_stft();
	this->onset_detection = OnsetDetection(this->stft->numBins());
}

_2011_PlRoSt::_2011_PlRoSt(const _2011_PlRoSt &that) : onset_detection(0)
{
	this->allocate_memory();
	this->initialize_stft();
	this->stft_frame = new Complex<float>[this->stft->numBins()];
	for (size_t k = 0; k < this->stft->numBins(); ++k)
	{
		this->stft_frame[k] = that.stft_frame[k];
	}
	this->onset_detection = that.onset_detection;
}

_2011_PlRoSt &_2011_PlRoSt::operator=(const _2011_PlRoSt &that)
{
	::operator delete(this->allocation_ptr);
	this->allocate_memory();
	this->initialize_stft();
	this->onset_detection = that.onset_detection;
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

int _2011_PlRoSt::operator()(float sample)
{
	if (!(*this->stft)(sample))
	{
		return 0;
	}

	for (size_t k = 0; k < this->stft->numBins(); ++k)
	{
		this->stft_frame[k] = this->stft->bin(k);
	}


}
