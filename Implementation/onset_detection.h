#ifndef IMPLEMENTIERUNG_ONSET_DETECTION_H
#define IMPLEMENTIERUNG_ONSET_DETECTION_H

#include <cstddef>
#include <Gamma/Types.h>

using namespace gam;

// TODO: unit test
// TODO: code style

/// Computes the complex spectral difference onset detection function of
/// [2004 Bello, Davies, Duxburry, Sandler - On the Use of Phase and Energy for Musical Onset Detection in the Complex Domain].
class OnsetDetector
{
	private:
		// number of frequency bins (also array size) of all "frames" used by this class
		size_t n_bins;

		// Two pointers to heap-allocated arrays of size `n_bins`.
		// [0] stores the previous frame
		// [1] stores the frame before that
		Complex<float> *prev_frames[2];

	public:
		explicit OnsetDetector(size_t n_bins);
		~OnsetDetector();

		/// Computes the next sample of the onset detection function as described in section IV of the paper.
		///
		/// \param stft_frame pointer to the input STFT frame
		/// \return said sample or NAN if `stft_frame` is null
		float next_sample(Complex<float> const *stft_frame);
};

#endif //IMPLEMENTIERUNG_ONSET_DETECTION_H
