#ifndef IMPLEMENTIERUNG_SHIFT_REGISTER_H
#define IMPLEMENTIERUNG_SHIFT_REGISTER_H

#include <cstddef>

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
		ShiftRegister();
		explicit ShiftRegister(size_t len);
		ShiftRegister(const ShiftRegister&);
		ShiftRegister& operator = (const ShiftRegister&);
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

		/// Pushes an array of items into the shift register.
		///
		/// This has the same effect as calling `push(item)` on every item in `buffer`.
		///
		/// \param buffer pointer to the array to push into the shift register
		/// \param buffer_len number of element to read from `buffer`
		void push_range(const float *buffer, size_t buffer_len);

		/// Returns the element at the given index.
		///
		/// ```
		/// result = shift_buffer[i];
		/// ```
		/// is equivalent to:
		/// ```
		/// shift_buffer.get_content(buffer);
		/// result = buffer[i];
		/// ```
		/// but you don't copy the enitry buffer.
		float operator[](size_t index);
};

#endif //IMPLEMENTIERUNG_SHIFT_REGISTER_H
