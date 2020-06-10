#ifndef IMPLEMENTIERUNG_CONSTANTS_H
#define IMPLEMENTIERUNG_CONSTANTS_H

#include <cmath>


// interval time at which ODF samples arrive in seconds (= 1 / odf sample rate)
const float ODF_SAMPLE_INTERVAL = 0.01161f;

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


#endif //IMPLEMENTIERUNG_CONSTANTS_H
