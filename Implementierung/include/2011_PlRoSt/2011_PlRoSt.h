#ifndef IMPLEMENTIERUNG_2011_PLROST_H
#define IMPLEMENTIERUNG_2011_PLROST_H

#include "2011_PlRoSt/2011_PlRoSt.h"

#include "2004_BeDaDuSa/onset_detection.h"
#include <Gamma/DFT.h>

using namespace gam;

// TODO: doc


class _2011_PlRoSt
{
	private:
		void *allocation_ptr;
		STFT *stft;
		Complex<float> *stft_frame;
		OnsetDetection onset_detection;

		// helper functions for the constructors
		void allocate_memory();
		void initialize_stft();

	public:
		explicit _2011_PlRoSt(float sample_rate);
		_2011_PlRoSt(const _2011_PlRoSt&);
		_2011_PlRoSt &operator = (const _2011_PlRoSt &);
		~_2011_PlRoSt();

		size_t get_n_bins() const;

		const Complex<float> *get_stft_frame() const;

		int operator ()(float sample);
};

#endif //IMPLEMENTIERUNG_2011_PLROST_H
