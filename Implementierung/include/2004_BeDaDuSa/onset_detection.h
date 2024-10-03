#ifndef IMPLEMENTIERUNG_ONSET_DETECTION_H
#define IMPLEMENTIERUNG_ONSET_DETECTION_H

#include <cstddef>
#include <Gamma/Types.h>

using namespace gam;

/// Computes the complex spectral difference onset detection function of
/// [2004 Bello, Davies, Duxburry, Sandler - On the Use of Phase and Energy for Musical Onset Detection in the Complex Domain].
class OnsetDetection
{
	private:
		// number of frequency bins (also array size) of all "frames" used by this class
		size_t n_bins;

		// previous frame
		Complex<float> *p_frame;

		// frame before the previous frame
		Complex<float> *pp_frame;

	public:
		explicit OnsetDetection(size_t n_bins);
		OnsetDetection(const OnsetDetection&);
		OnsetDetection& operator =(const OnsetDetection&);
		~OnsetDetection();

		/// Computes the next sample of the onset detection function as described in section IV of the paper.
		///
		/// \param stft_frame pointer to the input STFT frame
		/// \return said sample or NAN if `stft_frame` is null
		float operator ()(Complex<float> const *stft_frame);
};

#endif //IMPLEMENTIERUNG_ONSET_DETECTION_H
