#include "2009_DaPlSt/demo.h"

#include "2009_DaPlSt/2009_DaPlSt.h"
#include <cmath>
#include <cstring>
#include <Gamma/AudioIO.h>
#include <Gamma/SoundFile.h>
#include <iostream>
#include "misc.h"

using namespace gam;
using namespace std;


// represents the parsed command line args
struct Args
{
	char *executable;
	char *input_file;
	char *output_file;

	/// Parses the command line arguments.
	///
	/// If the arguments cannot be parsed, an error and usage message is printed
	/// and `executable` is set to NULL.
	Args(int argc, char **argv);
};

Args::Args(int argc, char **argv)
{
	if (argc == 3)
	{
		this->executable = argv[0];
		this->input_file = argv[1];
		this->output_file = argv[2];
	}
	else
	{
		const char *exe = argc >= 1 ? argv[0] : "./2009_DaPlSt_Demo";
		cerr << "Error: Invalid number of arguments." << endl;
		cerr << "Usage: $ " << exe << " [input file] [output file]" << endl;
		this->executable = nullptr;
	}
}


void audio_callback(AudioIOData& io)
{
	_2009_DaPlSt& beat_tracking = io.user<_2009_DaPlSt>();

	while (io())
	{
		float sample = io.in(0);

		beat_tracking.next(sample);
		// TODO: process result
	}
}

int demo(int argc, char **argv)
{
	Args args = Args(argc, argv);

	if (args.executable == nullptr)
	{
		return 1;
	}

	// use live input
	if (strcmp(args.input_file, "-") == 0)
	{
		AudioDevice dev = AudioDevice::defaultInput();
		_2009_DaPlSt beat_tracking = _2009_DaPlSt(dev.defaultSampleRate());
		AudioIO io(
			(int)round(ODF_SAMPLE_INTERVAL * dev.defaultSampleRate()),
			dev.defaultSampleRate(),
			&audio_callback,
			&beat_tracking,
			0,
			1
		);

		io.deviceIn(dev);
		io.print();

		io.start();
		cout << "Press ENTER to stop." << endl;
		cin.get();
		io.stop();
	}
	// use file input
	else
	{
		SoundFile file(args.input_file);
		file.openRead();

		_2009_DaPlSt beat_tracking = _2009_DaPlSt(file.frameRate());

		size_t buffer_len_frames = roundf(ODF_SAMPLE_INTERVAL * file.frameRate());
		size_t buffer_len_samples = buffer_len_frames * file.channels();
		float buffer[buffer_len_samples];
		int n_frames_read;

		while (0 != (n_frames_read = file.read(buffer, buffer_len_frames)))
		{
			for (int frame = 0; frame < n_frames_read; ++frame)
			{
				float mono_sample = avg(buffer + frame, file.channels());

				beat_tracking.next(mono_sample);
				// TODO: process result
			}
		}
	}

	return 0;
}
