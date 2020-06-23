#include "2001_ChNaUpVe/beat_this.h"
#include "2009_DaPlSt/2009_DaPlSt.h"
#include "2011_PlRoSt/2011_PlRoSt.h"
#include <fstream>
#include <Gamma/SoundFile.h>
#include <iostream>

using namespace gam;
using namespace std;


void test_2001_beat_this(const float *audio, size_t n_audio, ofstream *output_file)
{
	BeatThis tracker = BeatThis();

	*output_file << "timestamp_smp,timestamp_sec,beat_period_smp,beat_period_sec,tempo_bpm" << endl;

	for (size_t timestamp_smp = 0; timestamp_smp < n_audio; ++timestamp_smp)
	{
		if (tracker(audio[timestamp_smp]))
		{
			size_t beat_period_smp = tracker.get_beat_period();

			*output_file
				<< timestamp_smp << ","
				<< (timestamp_smp / 44100.0f) << ","
				<< beat_period_smp << ","
				<< (beat_period_smp / 44100.0f) << ","
				<< (60.0f * 44100.0f / beat_period_smp) << endl;
		}
	}
}

void test_2009_daplst(const float *audio, size_t n_audio, ofstream *output_file)
{
	_2009_DaPlSt tracker = _2009_DaPlSt(44100.0f);

	*output_file << "timestamp_smp,timestamp_sec,beat_period_smp,beat_period_sec,tempo_bpm,beat" << endl;

	size_t last_beat_time = -1;
	for (size_t timestamp_smp = 0; timestamp_smp < n_audio; ++timestamp_smp)
	{
		int result = tracker(audio[timestamp_smp]);
		size_t current_beat_time = tracker.get_current_beat_time();
		bool new_tempo = (result & 4) != 0;
		bool beat = current_beat_time != last_beat_time &&
		            current_beat_time == tracker.get_time();

		if (new_tempo || beat)
		{
			*output_file
				<< timestamp_smp << ","
				<< (timestamp_smp / 44100.0f) << ",";

			if (new_tempo)
			{
				float tempo = tracker.get_tempo();

				*output_file
					<< (60.0f * 44100.0f / tempo) << ","
					<< (60.0f / tempo) << ","
					<< tempo << ",";
			}
			else
			{
				*output_file << ",,,";
			}

			if (beat)
			{
				// make sure we don't go in here again, in the next iteration
				last_beat_time = current_beat_time;

				*output_file << "yes";
			}
			else
			{
				*output_file << "no";
			}

			*output_file << endl;
		}
	}
}

void test_2011_plrost(const float *audio, size_t n_audio, ofstream *output_file)
{
	_2011_PlRoSt tracker = _2011_PlRoSt();

	*output_file << "timestamp_smp,timestamp_sec,beat_period_smp,beat_period_sec,tempo_bpm,phase_smp,phase_sec,phase_deg,beat" << endl;

	size_t last_beat_time = -1;
	size_t last_tau = 0;
	size_t last_x = -1;

	for (size_t timestamp_smp = 0; timestamp_smp < n_audio; ++timestamp_smp)
	{
		bool new_tau_x = tracker(audio[timestamp_smp]) && (
			last_tau != tracker.get_current_tau() ||
			last_x != tracker.get_current_x()
		);
		size_t current_beat_time = tracker.get_time() / tracker.get_current_tau() * tracker.get_current_tau() + tracker.get_current_x();
		bool beat = current_beat_time != last_beat_time &&
		            current_beat_time == tracker.get_time();

		if (new_tau_x || beat)
		{
			*output_file
				<< timestamp_smp << ","
				<< (timestamp_smp / 44100.0f) << ",";

			if (new_tau_x)
			{
				last_tau = tracker.get_current_tau();
				last_x = tracker.get_current_x();

				// times coming from `tracker` are in ODF samples
				size_t beat_period_smp = tracker.get_current_tau() * 512;
				size_t phase_smp = tracker.get_current_x() * 512;

				*output_file
					<< beat_period_smp << ","
					<< beat_period_smp / 44100.0f << ","
					<< 60.0f * 44100.0f / beat_period_smp << ","
					<< phase_smp << ","
					<< (phase_smp / 44100.0f) << ","
					<< ((float)phase_smp / (float)beat_period_smp * 360.0f) << ",";
			}
			else
			{
				*output_file << ",,,,,,";
			}

			if (beat)
			{
				last_beat_time = current_beat_time;
				*output_file << "yes";
			}
			else
			{
				*output_file << "no";
			}

			*output_file << endl;
		}
	}
}

void test_generic(
	void (*test_function)(const float *, size_t, ofstream *),
	const float *audio, size_t n_audio, const char *f_name
)
{
	cout << "Writing " << f_name << " . . . ";
	cout.flush();

	ofstream file;
	file.open(f_name, ios::trunc);
	test_function(audio, n_audio, &file);
	file.close();

	cout << "done." << endl;
}

void test_all(const float *audio, size_t n_audio, const char *f_name_prefix)
{
	char output_f_name[strlen(f_name_prefix) + 19];

	strcpy(output_f_name, f_name_prefix);
	strcat(output_f_name, ".2001_BeatThis.csv");
	test_generic(&test_2001_beat_this, audio, n_audio, output_f_name);

	strcpy(output_f_name, f_name_prefix);
	strcat(output_f_name, ".2009_DaPlSt.csv");
	test_generic(&test_2009_daplst, audio, n_audio, output_f_name);

	strcpy(output_f_name, f_name_prefix);
	strcat(output_f_name, ".2011_PlRoSt.csv");
	test_generic(&test_2011_plrost, audio, n_audio, output_f_name);
}


int main(int argc, char **argv)
{
	if (argc == 1)
	{
		cerr << "Usage: $ " << argv[0] << " input0.wav input1.wav input2.wav ..." << endl;
		return 1;
	}

	for (int i = 1; i < argc; ++i)
	{
		char *f_name = argv[i];

		// open and load sound file
		SoundFile file(f_name);
		file.openRead();

		if (file.frameRate() != 44100)
		{
			cerr << "Skipping " << f_name
				 << ". Frame Rate should be 44100 Hz but is " << file.frameRate()
				 << "." << endl;
			continue;
		}

		if (file.channels() != 1)
		{
			cerr << "Skipping " << f_name
				 << ". Number of channels should be 1 but is " << file.channels() << "."
				 << endl;
			continue;
		}

		size_t n_buffer = file.channels() * file.frames();
		float buffer[n_buffer];
		file.readAll(buffer);
		file.close();

		// cut extension off of file name
		char *delimiter = strrchr(f_name, '.');
		if (delimiter != nullptr) *delimiter = '\0';

		// run tests
		test_all(buffer, n_buffer, f_name);
	}

	return 0;
}
