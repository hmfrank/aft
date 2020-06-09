#ifndef IMPLEMENTIERUNG_CONSTANTS_H
#define IMPLEMENTIERUNG_CONSTANTS_H

#include <cstddef>

// number of samples the STFT window is moved after each step
const size_t STFT_HOP_SIZE = 512;

// size of the STFT window in samples
const size_t STFT_WINDOW_SIZE = 2 * STFT_HOP_SIZE;

#endif //IMPLEMENTIERUNG_CONSTANTS_H
