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

// TODO: make accesses to the same variable from different threads thread-safe


// ========== CONSTANTS ========== //
// window title
const char *TITLE = "2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio";
// font for text inside the window
const char *FONT = "res/roboto.ttf";
// window width in px
const int WIDTH = 1000;
// expected sample rate of the input stream in Hz
const float SAMPLE_RATE = 44100;


// ========== GLOBAL VARIABLES ========== //
_2009_DaPlSt beat_tracking(SAMPLE_RATE);

// handles obtaining and processing of audio samples and runs until halt = true
thread audio_input_thread;

// window height
int HEIGHT;
// window handle
S2D_Window *window;

// text objects
S2D_Text *text_audio_input;
S2D_Text *text_spectrogram;
S2D_Text *text_odf;

// data to show on screen
ShiftRegister input_samples_max(WIDTH);
ShiftRegister input_samples_min(WIDTH);
float stft_max = 0;
ShiftRegister *stft_content;
ShiftRegister odf_samples(WIDTH);

// loop variable for endless-loop-threads
bool halt = false;


// ========== FUNCTIONS, STRUCTS, THE REST ========== //
void stdin_input_loop()
{
	const STFT *stft = beat_tracking.get_stft();
	ShiftRegister current_stft_frame(stft->sizeHop());
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
			for (int bin = 0; bin < stft->numBins(); ++bin)
			{
				stft_content[bin].push(stft->bin(bin).mag());
			}
			odf_samples.push(beat_tracking.get_odf_sample());
		}
	}
}

void update()
{
}

void render_audio_input(float top, float bottom)
{
	// top and bottom line
	S2D_DrawLine(
		0, top, WIDTH - 1, top, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	);
	S2D_DrawLine(
		0, bottom, WIDTH - 1, bottom, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	);

	// waveform
	float height = bottom - top;
	float y_mikkle = (bottom + top) / 2;

	for (int x = 0; x < WIDTH; ++x)
	{
		S2D_DrawLine(
			x, y_mikkle + input_samples_min[x] * height / 2,
			x, y_mikkle + input_samples_max[x] * height / 2,
			1,
			0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1
		);
	}

	// center line
	S2D_DrawLine(
		0, y_mikkle, WIDTH - 1, y_mikkle, 1,
		0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1
	);

	// text
	text_audio_input->y = top;
	S2D_DrawText(text_audio_input);
}

void render_spectrogram(float top, float bottom)
{
	// top and bottom line
	S2D_DrawLine(
		0, top, WIDTH - 1, top, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	);
	S2D_DrawLine(
		0, bottom, WIDTH - 1, bottom, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	);

	size_t n_bins = beat_tracking.get_stft()->numBins();

	float scale = 10 / stft_max;
	stft_max = 0;

	for (size_t bin = 0; bin < n_bins; ++bin)
	{
		for (size_t x = 0; x < WIDTH; ++x)
		{
			float intensity = stft_content[bin][x];
			float g = intensity * scale;

			if (intensity > stft_max)
			{
				stft_max = intensity;
			}

			// single pixel
			S2D_DrawLine(
				x, bottom - 1 - bin, x + 1, bottom - 1 - bin, 1,
				g, g, g, 1, g, g, g, 1, g, g, g, 1, g, g, g, 1
			);
		}
	}

	// text
	text_spectrogram->y = top;
	S2D_DrawText(text_spectrogram);
}

void render_time_grid()
{
	const float alpha = 0.25;
	float step = 1.0f / ODF_SAMPLE_INTERVAL;
	int i, x = 1;

	while ((x = WIDTH - (int)roundf(step * i)) > 0)
	{
		S2D_DrawLine(
			x, 0, x, HEIGHT, 1,
			1, 1, 1, alpha, 1, 1, 1, alpha, 1, 1, 1, alpha, 1, 1, 1, alpha
		);

		++i;
	}
}

void render_odf(float top, float bottom)
{
	// top and bottom line
	S2D_DrawLine(
		0, top, WIDTH - 1, top, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	);
	S2D_DrawLine(
		0, bottom, WIDTH - 1, bottom, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	);

	float height = bottom - top;
	// after this x value, another color is used to highlight the 6s analysis
	// frame of the tempo induction stage
	float threshold = WIDTH - 6.0f / ODF_SAMPLE_INTERVAL;
	float green;

	for (size_t x = 0; x < WIDTH; ++x)
	{
		green = x < threshold ? 1 : 0.5;

		S2D_DrawLine(
			x, bottom, x, bottom - odf_samples[x] * height / 2, 1,
			1, green, 0, 1, 1, green, 0, 1, 1, green, 0, 1, 1, green, 0, 1
		);
	}

	text_odf->y = top;
	S2D_DrawText(text_odf);
}

void render()
{
	size_t n_bins = beat_tracking.get_stft()->numBins();

	render_time_grid();
	render_audio_input(0, 200);
//	render_spectrogram(200, 200 + n_bins);
	render_odf(200, 400);
}

void init()
{
	// STFT shift registers
	size_t n_bins = beat_tracking.get_stft()->numBins();
	stft_content = new ShiftRegister[n_bins];
	assert(stft_content != nullptr);
	for (int bin = 0; bin < n_bins; ++bin)
	{
		stft_content[bin] = ShiftRegister(WIDTH);
	}

	// input thread
	audio_input_thread = thread(stdin_input_loop);

	// window
	HEIGHT = 400;
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
	text_audio_input->x = text_audio_input->color.r = text_audio_input->color.b = 0;
	text_audio_input->color.g = 1;

	text_spectrogram = S2D_CreateText(FONT, "Spectrogram", 20);
	assert(text_spectrogram != nullptr);
	text_spectrogram->x = 0;

	text_odf = S2D_CreateText(FONT, "Onset Detection Function", 20);
	assert(text_odf != nullptr);
	text_odf->x = text_odf->color.b = 0;
	text_odf->color.r = text_odf->color.g = 1;
}

void free()
{
	halt = true;

	S2D_FreeText(text_audio_input);
	S2D_FreeText(text_spectrogram);
	S2D_FreeText(text_odf);
	S2D_FreeWindow(window);
	audio_input_thread.join();
	delete[] stft_content;
}

void audio_callback(AudioIOData &io)
{
	_2009_DaPlSt &beat_tracking = io.user<_2009_DaPlSt>();

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
		(int) round(ODF_SAMPLE_INTERVAL * dev.defaultSampleRate()),
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


int demo(int argc, char **argv)
{
	Args args = Args(argc, argv);

	if (args.executable == nullptr)
	{
		return 1;
	}

	init();
	S2D_Show(window);
	free();

	return 0;
}