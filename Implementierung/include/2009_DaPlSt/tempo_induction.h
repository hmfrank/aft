#ifndef IMPLEMENTIERUNG_TEMPO_INDUCTION_H
#define IMPLEMENTIERUNG_TEMPO_INDUCTION_H

#include "2009_DaPlSt/constants.h"
#include <Gamma/Types.h>
#include "shift_register.h"

using namespace gam;

// TODO: unit test


// analysis frame size in seconds
const float ANALYSIS_FRAME_SIZE_S = 5.94432;
// analysis frame size in ODF-samples
const size_t ANALYSIS_FRAME_SIZE = roundf(ANALYSIS_FRAME_SIZE_S / ODF_SAMPLE_INTERVAL); // = 512

// after this many seconds a new tempo induction starts with the new analysis frame
const float ANALYSIS_FRAME_SHIFT_S = 1.48608;
// analysis frame shift in ODF-samples
const size_t ANALYSIS_FRAME_SHIFT = roundf(ANALYSIS_FRAME_SHIFT_S / ODF_SAMPLE_INTERVAL); // = 128

// inter-beat-interval of the preferred tempo in ODF samples
// see equation (8) in [2007 Davies, Plumbey - Context-Dependent Beat Tracking of Musical Audio]
const size_t BETA = roundf(60.0f / ODF_SAMPLE_INTERVAL / PREFERRED_TEMPO);


/// Implementation of the tempo induction part (Section 2.3.) of
/// [2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio].
/// Most details of this algorithm are described in
/// [2007 Davies, Plumbey - Context-Dependent Beat Tracking of Musical Audio].
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

		// Points to the block of memory that is allocated by this class
		float *allocation_ptr;

		// Points to an array of size `TTM_SIZE` that's allocated and freed in the constructor and destructor
		// of this class.
		//
		// For more information on the meaning of this array, see equations (9) through (12)
		// in [2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio].
		float *state_probabilities;

		// Points to the last modified analysis frame that is the input of the
		// auto-correlation.
		// This array has length `ANALYSIS_FRAME_SIZE`.
		float *modified_analysis_frame;

		// Points to the output buffer of the last auto correlation that was
		// performed on the modified analysis frame.
		// This array has length `ANALYSIS_FRAME_SIZE`.
		float *acf;

	public:
		TempoInduction();
		TempoInduction(const TempoInduction&);
		TempoInduction& operator =(const TempoInduction&);
		~TempoInduction();

		/// Returns a pointer to the output buffer of the last auto correlation
		/// that was performed on the modified analysis frame.
		///
		/// The array has length `ANALYSIS_FRAME_SIZE`.
		const float *get_acf() const;

		/// Returns a pointer to the last used modified analysis frame, which
		/// is the input of the auto-correlation.
		///
		/// The array is of length `ANALYSIS_FRAME_SIZE`.
		const float *get_modified_analysis_frame() const;

		/// Returns the number of new ODF samples since the last tempo analysis
		/// was made.
		size_t get_n_new_samples() const;

		/// Returns the current tempo estimate in BPM.
		float get_tempo() const;

		/// Consumes a sample of the onset detection function and updates the current tempo estimate if enough samples
		/// are there.
		///
		/// The tempo estimate is only updated every `ANALYSIS_FRAME_STEP` samples.
		///
		/// \param odf_sample next sample of the onset detection function
		/// \return true, iff the tempo estimate was updated
		bool operator ()(float odf_sample);

		/// Weighing function 'w_G' in
		/// [2007 Davies, Plumbey - Context-Dependent Beat Tracking of Musical Audio].
		static float comb_filter_weight(size_t tau);
};

#endif //IMPLEMENTIERUNG_TEMPO_INDUCTION_H
