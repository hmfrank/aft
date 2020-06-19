#ifndef IMPLEMENTIERUNG_CONSTANTS_H
#define IMPLEMENTIERUNG_CONSTANTS_H

#include <cstddef>
#include <cmath>

// sample rate in Hertz
const float SAMPLE_RATE = 44100.0;

// tempo update interval in samples
const size_t TEMPO_UPDATE_INTERVAL = 1.0 * SAMPLE_RATE;

// size of the hanning window used for smoothing
const size_t HANNING_WINDOW_SIZE = roundf(0.2f * SAMPLE_RATE);

// analysis frame in samples (1.49 seconds @ 44100Hz)
const size_t ANALYSIS_FRAME_SIZE = (1 << 16) - 2 * HANNING_WINDOW_SIZE;

// size of the analysis frame with padding
const size_t PADDED_AF_SIZE = ANALYSIS_FRAME_SIZE + 2 * HANNING_WINDOW_SIZE;
static_assert(PADDED_AF_SIZE % 2 == 0, "PADDED_AF_SIZE must be divisible by 2, so that the complex values that are saved in the buffer after the DFT fit in the buffer.");

// how many Hertz one FFT bin is worth
const float HZ_PER_BIN = SAMPLE_RATE / (float)PADDED_AF_SIZE;

// tempo range in BPM
const float MIN_TEMPO = 80.0;
const float MAX_TEMPO = 160.0;

// beatperiod range in samples
const size_t MIN_BP = 60.0 * SAMPLE_RATE / MAX_TEMPO;
const size_t MAX_BP = 60.0 * SAMPLE_RATE / MIN_TEMPO;

// When creating the comb filter bank, comb filters are created with beat
// periods ranging from `MIN_BP` to `MAX_BP` and a step size of `BP_STEP`.
const size_t BP_STEP = 512; // = 11.61 ms

const size_t N_COMB_FILTERS = (MAX_BP - MIN_BP) / BP_STEP;
static_assert(N_COMB_FILTERS > 0, "The tempo range must allow at least one comb filter.");

#endif //IMPLEMENTIERUNG_CONSTANTS_H
