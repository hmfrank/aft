#ifndef IMPLEMENTIERUNG_CONSTANTS_H
#define IMPLEMENTIERUNG_CONSTANTS_H

#include <cmath>

// expected input sample rate in Hz
const float SAMPLE_RATE = 44100.0;

// number of samples the STFT window is moved after each step
const size_t STFT_HOP_SIZE = 512;

// size of the STFT window in samples
const size_t STFT_WINDOW_SIZE = 2 * STFT_HOP_SIZE;


// interval time at which ODF samples arrive in seconds (= 1 / odf sample rate)
const float ODF_SAMPLE_INTERVAL = (float)STFT_HOP_SIZE / SAMPLE_RATE;

// analysis frame size in seconds
const float ANALYSIS_FRAME_SIZE_S = 6.0;

// analysis frame size in ODF samples
const size_t ANALYSIS_FRAME_SIZE =
	(size_t)roundf(ANALYSIS_FRAME_SIZE_S / ODF_SAMPLE_INTERVAL);


// minimum tempo in BPM
const float MIN_TEMPO = 80.0;

// maximum tempo in BPM
const float MAX_TEMPO = 160.0;

// sets strongest point of BPM weighing and the initial tempo estimate
const float PREFERRED_TEMPO = 120.0;

// minimum inter-beat-interval in ODF-samples
const size_t TAU_MIN = floorf(60.0f / ODF_SAMPLE_INTERVAL / MAX_TEMPO);

// maximum inter-beat-interval in ODF-samples
const size_t TAU_MAX = ceilf(60.0f / ODF_SAMPLE_INTERVAL / MIN_TEMPO);


// comb filter matrix width in ODF samples (= matrix pixels)
const size_t MATRIX_WIDTH = TAU_MAX;

// comb filter matrix height in ODF samples (= matrix pixels)
const size_t MATRIX_HEIGHT = TAU_MAX - TAU_MIN + 1;


// see equation (4) in the paper
const float ALPHA = 0.1;

// inter-beat-interval of the preferred tempo in ODF samples
// see equation (6) in the paper
const size_t BETA = roundf(60.0f / ODF_SAMPLE_INTERVAL / PREFERRED_TEMPO);

#endif //IMPLEMENTIERUNG_CONSTANTS_H
