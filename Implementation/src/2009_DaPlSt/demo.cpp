#include "2009_DaPlSt/demo.h"

#include "2009_DaPlSt/2009_DaPlSt.h"
#include <Gamma/AudioIO.h>
#include <iostream>
#include <simple2d.h>
#include "shift_register.h"
#include <thread>

using namespace gam;
using namespace std;


const char *TITLE = "2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio";
const int WIDTH = 1000;
const int HEIGHT = 500;


// represents the parsed command line args
struct Args
{
	char *executable;

	Args();

	/// Parses the command line arguments.
	///
	/// If the arguments cannot be parsed, an error and usage message is printed
	/// and `executable` is set to NULL.
	Args(int argc, char **argv);
};

Args::Args()
{
	this->executable = nullptr;
}

Args::Args(int argc, char **argv)
{
	this->executable = argv[0];
}


// current command line args
Args args;

// buffer of audio samples to show on the screen
ShiftRegister input_samples(WIDTH);

// loop variable for endless-loop-threads
bool halt = false;


void audio_callback(AudioIOData& io)
{
	_2009_DaPlSt& beat_tracking = io.user<_2009_DaPlSt>();

	cout << "callback" << endl;

	while (io())
	{
		float sample = io.in(0);

		cout << sample << endl;
		input_samples.push(sample);
		beat_tracking.next(sample);
	}
}

void setup_audio_input()
{
	AudioDevice::printAll();
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
}

void stdin_input_loop()
{
	while (!halt)
	{
		float sample;

		if (read(0, &sample, sizeof(sample)) != sizeof(sample))
		{
			break;
		}

		input_samples.push(sample);
	}
}

void update()
{

}

void render()
{
	float samples[WIDTH];
	input_samples.get_content(samples);

	for (int i = 0; i < WIDTH; ++i)
	{
		float sample = samples[i];
		float height = sample * HEIGHT;

		S2D_DrawLine(
			i, HEIGHT / 2, i, HEIGHT / 2 + height, 1,
			1, 1, 1, 1,
			1, 1, 1, 1,
			1, 1, 1, 1,
			1, 1, 1, 1
		);
	}
}

S2D_Window *create_window()
{
	S2D_Window *window = S2D_CreateWindow(
		TITLE,
		WIDTH,
		HEIGHT,
		update,
		render,
		0
	);
	window->viewport.mode = S2D_EXPAND;
	window->vsync = false;
}

int demo(int argc, char **argv)
{
	args = Args(argc, argv);

	if (args.executable == nullptr)
	{
		return 1;
	}

	// setup
	thread audio_input_thread(stdin_input_loop);
	S2D_Window *window = create_window();

	// run
	S2D_Show(window);

	// teardown
	halt = true;
	S2D_FreeWindow(window);
	audio_input_thread.join();

	return 0;
}
