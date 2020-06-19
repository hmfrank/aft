#ifndef IMPLEMENTIERUNG_BEAT_THIS_H
#define IMPLEMENTIERUNG_BEAT_THIS_H

#include "2001_ChNaUpVe/constants.h"
#include "shift_register.h"


class BeatThis
{
	private:
		float *hanning_window;

		ShiftRegister input_buffer;

		size_t time;

		size_t beat_period;

		size_t get_beat_period(float *padded_af);

	public:
		BeatThis();
		BeatThis(const BeatThis&);
		BeatThis& operator = (const BeatThis&);
		~BeatThis();

		size_t get_time() const;

		size_t get_beat_period() const;

		bool operator ()(float sample);
};

#endif //IMPLEMENTIERUNG_BEAT_THIS_H
