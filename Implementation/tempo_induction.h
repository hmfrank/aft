#ifndef IMPLEMENTIERUNG_TEMPO_INDUCTION_H
#define IMPLEMENTIERUNG_TEMPO_INDUCTION_H

#include <Gamma/Types.h>
#include "shift_register.h"

using namespace gam;

// TODO: unit test


/// Implementation of the tempo induction part (Section 2.3.) of
/// [2009 Davies+Plumbley+Stark - Real-time Beat-synchronous Analysis of Musical Audio].
///
/// The actual algorithm is in the `next_sample` method that is supposed to be called repeatedly for each new sample
/// of the onset detection function.
/// An instance of this class is needed to store some state between successive `next_sample`-calls.
class TempoInduction
{
	private:
		// stores the latest tempo estimate and is updated by `next_sample(...)`
		float current_tempo;

		// # odf-samples added to the input buffer since the last analysis
		size_t n_new_samples;

		ShiftRegister input_buffer;

		// Points to an array of size `TTM_SIZE` that's allocated and freed in the constructor and destructor
		// of this class.
		//
		// For more information on the meaning of this array, see equations (9) through (12)
		// in [2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio].
		float *state_probabilities;

	public:
		TempoInduction();
		~TempoInduction();

		/// Consumes a sample of the onset detection function and returns the current tempo estimate in BPM.
		///
		/// The tempo estimate is only updated every `ANALYSIS_FRAME_STEP` samples.
		/// Hence, you'll get the same return value as for the previous call, most of the time.
		///
		/// \param odf_sample next sample of the onset detection function
		/// \return current tempo estimate in BPM
		float next_sample(float odf_sample);
};

#endif //IMPLEMENTIERUNG_TEMPO_INDUCTION_H
