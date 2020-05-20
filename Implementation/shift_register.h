#ifndef IMPLEMENTIERUNG_SHIFT_REGISTER_H
#define IMPLEMENTIERUNG_SHIFT_REGISTER_H

#include <cstddef>

// TODO: generisch machen
// TODO: unit test
// TODO: code style

/// When you push a new item, it is added to the end of the buffer.
/// So the array you get from `get_content()` looks like this:
/// index:        0        1, 2, ...     len - 1
///        +-------------+-----------+-------------+
///        | oldest item |    ...    | newest item |
///        +-------------+-----------+-------------+
class ShiftRegister
{
	private:
		// index of the first item
		size_t start;

		// #items allocated for `data`
		size_t len;

		// head-allocated array
		float *data;

	public:
		explicit ShiftRegister(size_t len);
		~ShiftRegister();

		/// Writes the contents of the shift register into `buffer`.
		///
		/// \param buffer if null: nothing happens, otherwise: should point to an array of size >= get_len()
		void get_content(float *buffer) const;

		/// Returns the size of this shift register.
		size_t get_len() const;

		/// Pushes a new item to the end of the shift register, while the first one is removed.
		///
		/// \param value new item to append
		/// \return old item, that got removed
		float push(float value);
};

#endif //IMPLEMENTIERUNG_SHIFT_REGISTER_H
