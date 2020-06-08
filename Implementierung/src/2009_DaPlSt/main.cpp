#include "2009_DaPlSt/2009_DaPlSt.h"
#include <cassert>
#include "misc.h"
#include <simple2d.h>
#include "shift_register.h"
#include <thread>

using namespace gam;
using namespace std;


// ========== CONSTANTS ========== //
// window title
const char *TITLE = "2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio";
// font for text inside the window
const char *FONT = "res/roboto.ttf";
// window width in px
const size_t WIDTH = 1000;
// x coordinate of the present moment
const size_t X_PRESENT = WIDTH - TAU_MAX - 1;
// expected sample rate of the input stream in Hz
const float SAMPLE_RATE = 44100;


// ========== GLOBAL VARIABLES ========== //
_2009_DaPlSt beat_tracking(SAMPLE_RATE);

// handles obtaining and processing of audio samples and runs until halt = true
thread audio_input_thread;

// window height
size_t HEIGHT;
// window handle
S2D_Window *window;

// text objects
S2D_Text *text_acf;
S2D_Text *text_audio_input;
S2D_Text *text_modified_af;
S2D_Text *text_score_function;
S2D_Text *text_spectrogram;
S2D_Text *text_odf;

// data to show on screen
ShiftRegister input_samples_max(X_PRESENT);
ShiftRegister input_samples_min(X_PRESENT);
float stft_max = 0;
ShiftRegister *stft_content;
ShiftRegister odf_samples(X_PRESENT);
float score_max = 0;
ShiftRegister score_function(X_PRESENT);

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

		if (beat_tracking(sample) > 0)
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
			score_function.push(beat_tracking.get_beat_prediction()->get_current_score());
		}
	}
}

void update()
{
}

/// Returns the x coordinate of the current start of the analysis frame.
float get_x_analysis_frame_start()
{
	return (float)(
		X_PRESENT -
		ANALYSIS_FRAME_SIZE -
		beat_tracking.get_tempo_induction()->get_n_new_samples()
	);
}

/// Returns the x coordinate of the last pixel of the analysis frame + 1.
float get_x_analysis_frame_end()
{
	return (float)(
		X_PRESENT -
		beat_tracking.get_tempo_induction()->get_n_new_samples()
	);
}

void render_borders(float top, float bottom)
{
	S2D_DrawLine(
		0, top, WIDTH - 1, top, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	);
	S2D_DrawLine(
		0, bottom, WIDTH - 1, bottom, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	);
}

/// Draws a thick "present" line at `X_PRESENT` and 1-second-spaced thin grid
/// lines to the left.
/// Everything is drawn between the Y-coordinates `top` and `bottom`.
void render_time_grid(float top, float bottom)
{
	const float alpha = 0.25;
	float step = 1.0f / ODF_SAMPLE_INTERVAL;
	int i = 0, x = 0;

	while ((x = X_PRESENT - (int)roundf(step * i)) > 0)
	{
		S2D_DrawLine(
			x, bottom, x, top, 1,
			1, 1, 1, alpha, 1, 1, 1, alpha, 1, 1, 1, alpha, 1, 1, 1, alpha
		);

		++i;
	}

	// draw present line
	S2D_DrawLine(
		X_PRESENT, bottom, X_PRESENT, top, 2,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	);
}

/// Renders a time series with samples between 0 and 1.
///
/// If the samples of your time series are not in the range 0 to 1, you can use
/// the `gain` parameter to scale them before rendering.
///
/// \param top y coordinate of the top of the plot
/// \param bottom y coordinate of the bottom of the plot
/// \param buffer ptr to the time series
/// \param buffer_len # samples to plot
/// \param x_start x coordinate of the start (left edge) of the plot
/// \param gain multiplication factor of the time series before rendering
/// \param r red
/// \param g green
/// \param b blue
/// \param a alpha (opacity)
void render_time_series(
	float top, float bottom,
	const float *buffer, size_t buffer_len,
	float x_start, float gain,
	float r, float g, float b, float a
)
{
	for (size_t i = 0; i < buffer_len; ++i)
	{
		float x = i + x_start;

		S2D_DrawLine(
			x, bottom, x, bottom - buffer[i] * gain * (bottom - top), 1,
			r, g, b, a, r, g, b, a, r, g, b, a, r, g, b, a
		);
	}
}

void render_acf(float top, float bottom)
{
	render_borders(top, bottom);

	// render min max tempo lines
	S2D_DrawLine(
		TAU_MIN, bottom, TAU_MIN, top, 1,
		1, 1, 1, 0.25, 1, 1, 1, 0.25, 1, 1, 1, 0.25, 1, 1, 1, 0.25
	);
	S2D_DrawLine(
		TAU_MAX, bottom, TAU_MAX, top, 1,
		1, 1, 1, 0.25, 1, 1, 1, 0.25, 1, 1, 1, 0.25, 1, 1, 1, 0.25
	);

	// render comb filter weighing function
	float comb_gain = 0.8f / TempoInduction::comb_filter_weight(BETA);

	for (size_t i = 0; i < ANALYSIS_FRAME_SIZE; ++i)
	{
		float value = TempoInduction::comb_filter_weight(i);

		S2D_DrawLine(
			i, bottom, i, bottom - value * comb_gain * (bottom - top), 1,
			1, 1, 1, 0.25, 1, 1, 1, 0.25, 1, 1, 1, 0.25, 1, 1, 1, 0.25
		);
	}

	// render acf
	const float *acf = beat_tracking.get_tempo_induction()->get_acf();
	float r = 0;
	float acf_gain = 1.0f / max(acf, TAU_MAX * 4 + 3);

	for (size_t i = 0; i < ANALYSIS_FRAME_SIZE; ++i)
	{
		r = i > TAU_MAX * 4 + 3 ? 1 : 0;

		S2D_DrawLine(
			i, bottom,
			i, max(top, bottom - acf[i] * acf_gain * (bottom - top)),
			1,
			r, 1, 1, 1, r, 1, 1, 1, r, 1, 1, 1, r, 1, 1, 1
		);
	}

	// render comb filter
	size_t tau = 60.0f / ODF_SAMPLE_INTERVAL / beat_tracking.get_tempo();
	comb_gain *= TempoInduction::comb_filter_weight(tau);

	for (ssize_t p = 1; p <= 4; ++p)
	{
		for (ssize_t v = 1 - p; v <= p - 1; ++v)
		{
			ssize_t lag = tau * p + v;
			float height = comb_gain / (float)(2 * p - 1);

			S2D_DrawLine(
				lag, bottom, lag, bottom - height * (bottom - top), 1,
				1, 0, 0, 0.5, 1, 0, 0, 0.5, 1, 0, 0, 0.5, 1, 0, 0, 0.5
			);
			// this->acf[lag] * weight / (float) (2 * p - 1);
		}
	}

	// render text
	text_acf->y = (int)top;
	S2D_DrawText(text_acf);

	char string[3][128];
	snprintf(
		string[0], 128,
		"Tempo Range: %.1f - %.1f BPM",
		MIN_TEMPO, MAX_TEMPO
	);
	snprintf(
		string[1], 128,
		"Tempo: %.1f BPM",
		beat_tracking.get_tempo()
	);
	snprintf(
		string[2], 128,
		"Beat Interval: %lu ODF Samples",
		tau
	);

	for (int i = 0; i < 3; ++i)
	{
		S2D_Text *text = S2D_CreateText(FONT, string[i], 20);
		if (text != nullptr)
		{
			text->x = ANALYSIS_FRAME_SIZE;
			text->y = (int)top + 25 * i;
			text->color.r = text->color.g = text->color.b = 1;
			S2D_DrawText(text);
			S2D_FreeText(text);
		}
	}
}

void render_audio_input(float top, float bottom)
{
	render_time_grid(top, bottom);
	render_borders(top, bottom);

	// waveform
	float height = bottom - top;
	float y_mikkle = (bottom + top) / 2;

	for (int x = 0; x < X_PRESENT; ++x)
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
		0, y_mikkle, X_PRESENT - 1, y_mikkle, 1,
		0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1
	);

	// text
	text_audio_input->y = (int)top;
	S2D_DrawText(text_audio_input);
}

void render_modified_analysis_frame(float top, float bottom)
{
	render_time_grid(top, bottom);
	render_borders(top, bottom);

	const float *mod_af = beat_tracking.get_tempo_induction()->get_modified_analysis_frame();

	render_time_series(
		top, bottom,
		mod_af, ANALYSIS_FRAME_SIZE,
		get_x_analysis_frame_start(), 1,
		0, 0.5, 1.0, 1.0
	);

	text_modified_af->y = (int)top;
	S2D_DrawText(text_modified_af);
}

void render_score_function(float top, float bottom)
{
	render_time_grid(top, bottom);
	render_borders(top, bottom);

	float gain = 1.0f / (score_max > 0 ? score_max : 1);
	score_max = 0;

	// past score function
	for (size_t x = 0; x < X_PRESENT; ++x)
	{
		float score = score_function[x];

		if (score > score_max)
		{
			score_max = score;
		}

		S2D_DrawLine(
			x, bottom, x, bottom - score * gain * (bottom - top), 1,
			1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1
		);
	}

	// future score function
	const float *future = beat_tracking.get_beat_prediction()->get_future_score();
	size_t future_len = beat_tracking.get_beat_prediction()->get_beat_period();

	render_time_series(
		top, bottom,
		future, future_len,
		X_PRESENT, gain,
		1, 0, 0.5, 1
	);

	// next beat prediction
	float relative_next_beat_time = beat_tracking.get_next_beat_time() - beat_tracking.get_time();
	float x = X_PRESENT - 1 + relative_next_beat_time;
	S2D_DrawLine(
		x, bottom, x, top, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	);

	// text
	text_score_function->y = (int)top;
	S2D_DrawText(text_score_function);

	// draw present line, again
	S2D_DrawLine(
		X_PRESENT, bottom, X_PRESENT, top, 2,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	);
}

void render_spectrogram(float top, float bottom)
{
	render_borders(top, bottom);

	size_t n_bins = beat_tracking.get_stft()->numBins();

	float scale = 10 / (stft_max > 0 ? stft_max : 1);
	stft_max = 0;

	for (size_t bin = 0; bin < n_bins; ++bin)
	{
		for (size_t x = 0; x < X_PRESENT; ++x)
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
	text_spectrogram->y = (int)top;
	S2D_DrawText(text_spectrogram);
}

void render_odf(float top, float bottom)
{
	render_time_grid(top, bottom);
	render_borders(top, bottom);

	float height = bottom - top;
	// between `af_start` and `af_end`, another color is used to highlight the
	// 6s analysis frame of the tempo induction stage
	float af_start = get_x_analysis_frame_start();
	float af_end = get_x_analysis_frame_end();
	float green;

	for (size_t x = 0; x < X_PRESENT; ++x)
	{
		green = (x >= af_start) && (x < af_end) ? 0.5 : 1;

		S2D_DrawLine(
			x, bottom, x, bottom - odf_samples[x] * height / 2, 1,
			1, green, 0, 1, 1, green, 0, 1, 1, green, 0, 1, 1, green, 0, 1
		);
	}

	text_odf->y = (int)top;
	S2D_DrawText(text_odf);
}

void render()
{
	size_t n_bins = beat_tracking.get_stft()->numBins();

//	render_spectrogram(0, n_bins);
	render_audio_input(0, 200);
	render_odf(200, 400);
	render_modified_analysis_frame(400, 600);
	render_score_function(600, 800);
	render_acf(800, 1000);
}

void init()
{
	// STFT shift registers
	size_t n_bins = beat_tracking.get_stft()->numBins();
	stft_content = new ShiftRegister[n_bins];
	assert(stft_content != nullptr);
	for (int bin = 0; bin < n_bins; ++bin)
	{
		stft_content[bin] = ShiftRegister(X_PRESENT);
	}

	// input thread
	audio_input_thread = thread(stdin_input_loop);

	// window
	HEIGHT = 1000;
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
	text_acf = S2D_CreateText(FONT, "ACF of Modified Analysis Frame + Comb Filter", 20);
	assert(text_acf != nullptr);
	text_acf->x = text_acf->color.r = 0;
	text_acf->color.g = text_acf->color.b = 1;

	text_audio_input = S2D_CreateText(FONT, "Audio Input", 20);
	assert(text_audio_input != nullptr);
	text_audio_input->x = text_audio_input->color.r = text_audio_input->color.b = 0;
	text_audio_input->color.g = 1;

	text_modified_af = S2D_CreateText(FONT, "Modified Analysis Frame", 20);
	assert(text_modified_af != nullptr);
	text_modified_af->x = text_modified_af->color.r = 0;
	text_modified_af->color.g = 0.5;
	text_modified_af->color.b = 1;

	text_score_function = S2D_CreateText(FONT, "Score Function", 20);
	assert(text_score_function != nullptr);
	text_score_function->x = text_score_function->color.g = text_score_function->color.b = 0;
	text_score_function->color.r = 1;

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

	S2D_FreeText(text_acf);
	S2D_FreeText(text_audio_input);
	S2D_FreeText(text_modified_af);
	S2D_FreeText(text_score_function);
	S2D_FreeText(text_spectrogram);
	S2D_FreeText(text_odf);
	S2D_FreeWindow(window);
	audio_input_thread.join();
	delete[] stft_content;
}


int main(int argc, char **argv)
{
	init();
	S2D_Show(window);
	free();

	return 0;
}