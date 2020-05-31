#include "2009_DaPlSt/demo.h"

#include "2009_DaPlSt/2009_DaPlSt.h"
#include <cassert>
#include <Gamma/AudioIO.h>
#include <iostream>
#include "misc.h"
#include <simple2d.h>
#include "shift_register.h"
#include <thread>

using namespace gam;
using namespace std;


const char *TITLE = "2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio";
const char *FONT = "res/roboto.ttf";
const int WIDTH = 1000;
const int HEIGHT = 500;
const float SAMPLE_RATE = 44100;


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

thread audio_input_thread;

// main window
S2D_Window *window;

// text objects
S2D_Text *text_audio_input;

// data to show on screen
ShiftRegister input_samples_max(WIDTH);
ShiftRegister input_samples_min(WIDTH);

// loop variable for endless-loop-threads
bool halt = false;


void stdin_input_loop()
{
	_2009_DaPlSt beat_tracking(SAMPLE_RATE);
	ShiftRegister current_stft_frame(beat_tracking.get_stft()->sizeHop());

	float sample;

	while (!halt && (read(0, &sample, sizeof(sample)) == sizeof(sample)))
	{
		current_stft_frame.push(sample);

		if (beat_tracking.next(sample) > 0)
		{
			float samples[current_stft_frame.get_len()];
			current_stft_frame.get_content(samples);

			input_samples_min.push(min(samples, current_stft_frame.get_len()));
			input_samples_max.push(max(samples, current_stft_frame.get_len()));
		}
	}
}

void update()
{
}

void render()
{
	float samples_min[WIDTH];
	float samples_max[WIDTH];

	input_samples_min.get_content(samples_min);
	input_samples_max.get_content(samples_max);

	for (int i = 0; i < WIDTH; ++i)
	{
		S2D_DrawLine(
			i, HEIGHT / 2 + samples_min[i] * HEIGHT / 2, i, HEIGHT / 2 + samples_max[i] * HEIGHT / 2, 1,
			1, 1, 1, 1,
			1, 1, 1, 1,
			1, 1, 1, 1,
			1, 1, 1, 1
		);
	}

	S2D_DrawText(text_audio_input);
}

void init()
{
	// input thread
	audio_input_thread = thread(stdin_input_loop);

	// window
	window = S2D_CreateWindow(
		TITLE,
		WIDTH,
		HEIGHT,
		update,
		render,
		0
	);
	assert(window != nullptr);

	window->viewport.mode = S2D_EXPAND;
	window->vsync = false;

	// text objects
	text_audio_input = S2D_CreateText(FONT, "Audio Input", 20);
	assert(text_audio_input != nullptr);
	text_audio_input->x = text_audio_input->y = 0;
}

void free()
{
	halt = true;

	S2D_FreeText(text_audio_input);
	S2D_FreeWindow(window);
	audio_input_thread.join();
}

void audio_callback(AudioIOData& io)
{
	_2009_DaPlSt& beat_tracking = io.user<_2009_DaPlSt>();

	cout << "callback" << endl;

	while (io())
	{
		float sample = io.in(0);

		cout << sample << endl;
		input_samples_max.push(sample);
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

int demo(int argc, char **argv)
{
	args = Args(argc, argv);

	if (args.executable == nullptr)
	{
		return 1;
	}

	init();
	S2D_Show(window);
	free();

	return 0;
}