#ifndef IMPLEMENTIERUNG_BEAT_PREDICTION_H
#define IMPLEMENTIERUNG_BEAT_PREDICTION_H

#include <cstdio>
#include "shift_register.h"


/// Implementation of the beat prediction part (Section 2.2.) of
/// [2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio].
///
/// The actual algorithm is in the ()-operator that is supposed to be called repeatedly for each new sample
/// of the onset detection function.
/// An instance of this class is needed to store some state between successive ()-operator-calls.
class BeatPrediction
{
	private:
		// points to the block of memory that is allocated by this class
		float *allocation_ptr;

		// weighting function (in the form of a lookup table) for past score function samples
		// equation (3) in [2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio]
		float *past_weighting;

		// weighting function (in the form of a lookup table) for future score function samples
		// equation (5) in [2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio]
		float *future_weighting;

		// past score function samples
		ShiftRegister past_score;

		// current score function sample C*(m)
		float current_score;

		// future score function samples
		float *future_score;

		// inter beat interval of the current tempo estimate in ODF samples
		size_t beat_period;

		// current time in ODF samples
		size_t time;

		// time of the current beat prediction
		// (can be in both the future or the past)
		size_t current_beat_time;

		// returns the score function at the given index, where 0 is the present moment and negative value are the past
		float score_function(ssize_t index);

		// Helper function for `next_prediction()`.
		// The name stands for equation 2 right hand side and refers to
		// [2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio].
		float eq2rhs(ssize_t m);

	public:
		BeatPrediction();
		BeatPrediction(const BeatPrediction&);
		BeatPrediction& operator = (const BeatPrediction&);
		~BeatPrediction();

		/// Returns the inter beat interval of the current tempo estimate in
		/// ODF samples.
		float get_beat_period() const;

		/// Returns the time of the current beat prediction
		size_t get_current_beat_time() const;

		/// Returns the current value of the score function.
		float get_current_score() const;

		/// Returns a pointer to the future score function estimate.
		///
		/// The returned pointer points to memory that belongs to this object,
		/// so don't free it or do anything else with it, other than reading.
		/// The array has length `this->get_beat_period()` and the maximum
		/// length is `TAU_MAX + 1`.
		const float *get_future_score() const;

		/// Returns the current time in ODF samples
		size_t get_time() const;

		/// Updates the tempo used for beat prediction.
		///
		/// \param tempo new tempo in BPM
		void set_tempo(float tempo);

		/// Consumes a sample of the onset detection function and returns true
		/// iff the current beat time was updated.
		///
		/// \param odf_sample next sample of the onset detection function
		bool operator ()(float odf_sample);
};

#endif //IMPLEMENTIERUNG_BEAT_PREDICTION_H
