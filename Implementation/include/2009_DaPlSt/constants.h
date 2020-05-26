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



#endif //IMPLEMENTIERUNG_CONSTANTS_H
