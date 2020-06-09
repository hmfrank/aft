#ifndef IMPLEMENTIERUNG_2011_PLROST_H
#define IMPLEMENTIERUNG_2011_PLROST_H

#include "2011_PlRoSt/2011_PlRoSt.h"

#include "2004_BeDaDuSa/onset_detection.h"
#include <Gamma/DFT.h>
#include "shift_register.h"

using namespace gam;


/// Implementation of the algorithm described in
/// [2011 Plumbley, Robertson, Stark - Real-time Visual Beat Tracking Using a Comb Filter Matrix].
///
/// How to use this class:
/// 1. initialize an instance with the sample rate of your audio stream
/// 2. for each sample in your audio stream:
/// 2.1. call `your_2011_PlRoSt_instance(sample)`
/// 2.2. check the return value
class _2011_PlRoSt
{
	private:
		OnsetDetection onset_detection;

		// analysis frame 2 determine the median of the onset detection function
		ShiftRegister analysis_frame;

		// median of `analysis_frame`
		float af_median;

		// elapsed time in ODF samples
		size_t time;

		// points to the block of memory that was allocated for this instance
		void *allocation_ptr;

		// object to compute the short time fourier transform
		STFT *stft;

		// latest STFT frame (length: `stft->numBins()`)
		Complex<float> *stft_frame;

		// current comb filter matrix
		// array length: MATRIX_WIDTH * MATRIX_HEGIHT
		// access to coordinate x, y: matrix[y * MATRIX_WIDTH + x];
		float *matrix;

		// helper functions for the constructors
		void allocate_memory();
		void initialize_stft();

	public:
		explicit _2011_PlRoSt();
		_2011_PlRoSt(const _2011_PlRoSt&);
		_2011_PlRoSt &operator = (const _2011_PlRoSt &);
		~_2011_PlRoSt();

		size_t get_n_bins() const;

		const Complex<float> *get_stft_frame() const;

		int operator ()(float sample);
};

#endif //IMPLEMENTIERUNG_2011_PLROST_H
