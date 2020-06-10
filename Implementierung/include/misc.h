#ifndef IMPLEMENTIERUNG_MISC_H
#define IMPLEMENTIERUNG_MISC_H

#include <cstddef>

/// Computes the arithmetic mean (average) of the given array.
float avg(float const *buffer, size_t buffer_len);

/// Returns the maximum value in the given array or NaN if the array is empty.
float max(float const *buffer, size_t buffer_len);

/// Returns the greater value of the two given numbers.
float max(float, float);

/// Returns the minimum value in the given array or NaN if the array is empty.
float min(float const *buffer, size_t buffer_len);

/// Returns the smaller value of the two given numbers.
float min(float, float);

#endif //IMPLEMENTIERUNG_MISC_H
