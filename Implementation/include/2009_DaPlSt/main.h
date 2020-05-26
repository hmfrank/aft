#ifndef IMPLEMENTIERUNG_MAIN_H
#define IMPLEMENTIERUNG_MAIN_H

#include "2009_DaPlSt/beat_prediction.h"
#include "2009_DaPlSt/onset_detection.h"
#include "2009_DaPlSt/tempo_induction.h"
#include <Gamma/DFT.h>

using namespace gam;

// TODO: unit test


/// Implementation of the algorithm described in
/// [2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio].
///
/// How to use this class:
/// 1. initialize an instance with the sample rate of your audio stream
/// 2. for each sample in your audio stream:
/// 2.1. call `next(sample)`
/// 2.2. check the return value of `next()` and read the next beat prediction and tempo estimate
class _2009_DaPlSt
{
	private:
		size_t time;

		//
		size_t next_beat;

		STFT stft;
		OnsetDetector onset_detection;
		TempoInduction tempo_induction;
		BeatPrediction beat_prediction;

	public:
		/// \param sample_rate Rate of the audio samples passed to `next()` in Herz.
		///     Recommended Sample Rate: 44100 or any half or double (22050, 11025, 88200, etc.)
		explicit _2009_DaPlSt(float sample_rate);

		/// Returns current stream time in onset detection function samples.
		///
		/// One unit is worth `ODF_SAMPLE_INTERVAL` seconds.
		size_t get_time() const;

		/// Returns the predicted time of the next beat in ODF samples.
		///
		/// One unit is worth `ODF_SAMPLE_INTERVAL` seconds.
		size_t get_next_beat_time() const;

		/// Returns the current tempo estimate in BPM.
		float get_tempo() const;

		/// Consumes and processes the next audio sample of the input stream and updates the predicted beat time and
		/// tempo estimate if enough samples are consumed.
		///
		/// \return There are three return codes with different meanings:
		///     0: no updates
		///     1: next beat prediction got updated (check with `get_next_beat_time()`)
		///     2: next beat prediction and tempo estimate got updated (check with `get_next_beat_time()` and `get_tempo()`)
		int next(float sample);
};

#endif //IMPLEMENTIERUNG_MAIN_H
