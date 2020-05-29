#ifndef IMPLEMENTIERUNG_MISC_H
#define IMPLEMENTIERUNG_MISC_H

#include <cstddef>

/// Computes the arithmetic mean (average) of the given array.
///
/// \param buffer pointer to the first element of the array
/// \param buffer_len array length
/// \return average value of the given array
float avg(float const *buffer, size_t buffer_len);

#endif //IMPLEMENTIERUNG_MISC_H
