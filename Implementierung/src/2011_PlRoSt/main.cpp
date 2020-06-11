#include "2011_PlRoSt/2011_PlRoSt.h"
#include "2011_PlRoSt/constants.h"
#include <cassert>
#include <cstdio>
#include "misc.h"
#include <simple2d.h>
#include "shift_register.h"
#include <thread>

using namespace std;

// ========== CONSTANTS ========== //

// window title
const char *TITLE = "2011 Plumbley, Robertson, Stark - Real-Time Visual Beat Tracking Using a Comb Filter Matrix";

// font for text inside the window
const char *FONT = "res/roboto.ttf";

// size of the matrix pixels in screen pixels
const int MATRIX_PX_SIZE = 7;

// window size in pixels
const int WIDTH = 1000;
const int HEIGHT = 830 + MATRIX_HEIGHT * MATRIX_PX_SIZE + 30;

// colors
struct Color { float r, g, b, a; };
#define COMMA_SPLIT_COLOR(col) (col).r, (col).g, (col).b, (col).a
#define COMMA_SPLIT_COLOR_4(col) COMMA_SPLIT_COLOR(col), COMMA_SPLIT_COLOR(col), COMMA_SPLIT_COLOR(col), COMMA_SPLIT_COLOR(col)
#define SET_TEXT_COL(textptr, col) ((textptr)->color.r = (col).r, (textptr)->color.g = (col).g, (textptr)->color.b = (col).b, (textptr)->color.a = (col).a)

const Color C_RED = Color{ .r = 1, .g = 0, .b = 0, .a = 1 };
const Color C_ORANGE = Color{ .r = 1, .g = 0.5, .b = 0, .a = 1 };
const Color C_YELLOW = Color{ .r = 1, .g = 1, .b = 0, .a = 1 };
const Color C_GREEN = Color{ .r = 0, .g = 1, .b = 0, .a = 1 };
const Color C_CYAN = Color{ .r = 0, .g = 1, .b = 1, .a = 1};
const Color C_WHITE = Color{ .r = 1, .g = 1, .b = 1, .a = 1 };
const Color C_GRID_LINES = Color{ .r = 1, .g = 1, .b = 1, .a = 0.25 };

// ========== APP CLASS ========== //
struct Bounds
{
	float left, top, right, bottom;

	float width() const { return right - left; }
	float height() const { return bottom - top; }
	float y_miggle() const { return (bottom + top) / 2; }
};

class MyApp
{
	private:
		// BEAT TRACKING
		_2011_PlRoSt beat_tracking;

		// RENDER DATA
		ShiftRegister input_samples_min;
		ShiftRegister input_samples_max;
		ShiftRegister odf_samples;
		ShiftRegister pp_odf_samples;
		S2D_Text *text_audio_input;
		S2D_Text *text_help;
		S2D_Text *text_odf;
		S2D_Text *text_pp_odf;
		S2D_Text *text_x_matrix;
		S2D_Text *text_y_matrix;

		// GUI
		bool halt;
		S2D_Window *window;
		thread input_thread;

		void render_audio_input(Bounds b, Color c);

		void render_beat_indicator(Bounds b, Color c);

		void render_help(Bounds b, Color c);

		void render_info_box(Bounds b, Color c_current, Color c_new);

		void render_matrix(bool x, Bounds b, Color c, Color c_now, Color *c_current, Color *c_new);

		void render_odf(Bounds b, Color c, Color c_af);

		void render_pp_odf(Bounds b, Color c);

		void render_x_matrix(Bounds b, Color c, Color c_now);

		void render_y_matrix(Bounds b, Color c, Color c_now, Color c_current, Color c_new);

		static void input_thread_main();

		static void on_key(S2D_Event e);

		static void render();

		static void render_grid_lines(Bounds b, Color c);

	public:
		static void init();

		static void free();

		static void run();
};

MyApp app;

void MyApp::render_audio_input(Bounds b, Color c)
{
	// grid lines
	MyApp::render_grid_lines(b, C_GRID_LINES);

	// waveform
	float height = b.height();
	float y_miggle = b.y_miggle();
	int n = this->input_samples_min.get_len();

	for (int i = n - (int)b.width(); i < n; ++i)
	{
		float x = (float)(i - n) + b.right;
		float y0 = y_miggle + this->input_samples_min[i] * height / 2;
		float y1 = y_miggle + this->input_samples_max[i] * height / 2;

		S2D_DrawLine(
			x, y0, x, y1, 1,
			COMMA_SPLIT_COLOR_4(c)
		);
	}

	// zero line
	S2D_DrawLine(
		b.left, y_miggle, b.right, y_miggle, 1,
		COMMA_SPLIT_COLOR_4(c)
	);

	// text
	SET_TEXT_COL(app.text_audio_input, c);
	app.text_audio_input->x = b.left;
	app.text_audio_input->y = b.top;
	S2D_DrawText(app.text_audio_input);
}

void MyApp::render_beat_indicator(Bounds b, Color c)
{
	size_t dif =
		this->beat_tracking.get_time() % this->beat_tracking.get_current_tau() -
		this->beat_tracking.get_current_x();

	if (dif >= 0 && dif <= 10)
	{
		S2D_DrawQuad(
			b.left, b.top, COMMA_SPLIT_COLOR(c),
			b.right, b.top, COMMA_SPLIT_COLOR(c),
			b.right, b.bottom, COMMA_SPLIT_COLOR(c),
			b.left, b.bottom, COMMA_SPLIT_COLOR(c)
		);
	}
}

void MyApp::render_help(Bounds b, Color c)
{
	this->text_help->x = b.left;
	this->text_help->y = b.top;
	SET_TEXT_COL(this->text_help, c);
	S2D_DrawText(this->text_help);
}

void MyApp::render_info_box(Bounds b, Color c_current, Color c_new)
{
	size_t current_tau = this->beat_tracking.get_current_tau();
	float current_tempo = 60.0f / ODF_SAMPLE_INTERVAL / (float)current_tau;
	size_t current_x = this->beat_tracking.get_current_x();
	float current_phi = (float)current_x / current_tau * 360.0f;
	size_t new_tau = this->beat_tracking.get_new_tau();
	float new_tempo = 60.0f / ODF_SAMPLE_INTERVAL / (float)new_tau;
	size_t new_x = this->beat_tracking.get_new_x();
	float new_phi = (float)new_x / new_tau * 360.0f;

	char text_buffer[4][1024];

	snprintf(
		text_buffer[0], 1024, "Current Tau: %lu (IBI = %.4g ms, Tempo = %.4g BPM)",
		current_tau, 60000.0f / current_tempo, current_tempo
	);
	snprintf(
		text_buffer[1], 1024, "Current X: %lu (Phase = %.4g deg.)",
		current_x, current_phi
	);
	snprintf(
		text_buffer[2], 1024, "New Tau: %lu (IBI = %.4g ms, Tempo = %.4g BPM)",
		new_tau, 60000.0f / new_tempo, new_tempo
	);
	snprintf(
		text_buffer[3], 1024, "New X: %lu (Phase = %.4g deg.)",
		new_x, new_phi
	);

	S2D_Text *text;

	for (int i = 0; i < 4; ++i)
	{
		text = S2D_CreateText(FONT, text_buffer[i], 20);

		if (text != nullptr)
		{
			text->x = b.left;
			text->y = b.top + 30.0f * (float)i;
			SET_TEXT_COL(text, i < 2 ? c_current : c_new);

			S2D_DrawText(text);
			S2D_FreeText(text);
		}
	}
}

void MyApp::render_odf(Bounds b, Color c, Color c_af)
{
	// grid lines
	MyApp::render_grid_lines(b, C_GRID_LINES);

	//waveform
	float median = this->beat_tracking.get_analysis_frame_median();
	float height = b.height();
	int n = this->odf_samples.get_len();

	for (int i = n - (int)b.width(); i < n; ++i)
	{
		float x = (float)(i - n) + b.right;
		float y = b.bottom - this->odf_samples[i] * height;
		float y_median = b.bottom - median * height;
		Color col = i <= n - ANALYSIS_FRAME_SIZE ? c : c_af;

		S2D_DrawLine(
			x, b.bottom, x, y, 1,
			COMMA_SPLIT_COLOR_4(col)
		);
	}

	// zero line
	S2D_DrawLine(
		b.left, b.bottom, b.right - ANALYSIS_FRAME_SIZE, b.bottom, 1,
		COMMA_SPLIT_COLOR_4(c)
	);
	S2D_DrawLine(
		b.right - ANALYSIS_FRAME_SIZE, b.bottom, b.right, b.bottom, 1,
		COMMA_SPLIT_COLOR_4(c_af)
	);

	// median line
	float y = b.bottom - height * median;
	S2D_DrawLine(
		b.right - ANALYSIS_FRAME_SIZE, y, b.right, y, 1,
		COMMA_SPLIT_COLOR_4(C_WHITE)
	);

	// text
	SET_TEXT_COL(app.text_odf, c);
	app.text_odf->x = b.left;
	app.text_odf->y = b.top;
	S2D_DrawText(app.text_odf);
}

void MyApp::render_pp_odf(Bounds b, Color c)
{

	// grid lines
	MyApp::render_grid_lines(b, C_GRID_LINES);

	//waveform
	float height = b.height();
	int n = this->pp_odf_samples.get_len();

	for (int i = n - (int)b.width(); i < n; ++i)
	{
		float x = (float)(i - n) + b.right;
		float y = b.bottom - this->pp_odf_samples[i] * height;

		S2D_DrawLine(
			x, b.bottom, x, y, 1,
			COMMA_SPLIT_COLOR_4(c)
		);
	}

	// zero line
	S2D_DrawLine(
		b.left, b.bottom, b.right, b.bottom, 1,
		COMMA_SPLIT_COLOR_4(c)
	);

	// text
	SET_TEXT_COL(app.text_pp_odf, c);
	app.text_pp_odf->x = b.left;
	app.text_pp_odf->y = b.top;
	S2D_DrawText(app.text_pp_odf);
}

void MyApp::render_matrix(bool x, Bounds b, Color c, Color c_now, Color *c_current, Color *c_new)
{
	const float *matrix = x ?
						  this->beat_tracking.get_x_matrix() :
						  this->beat_tracking.get_y_matrix();
	const float scaling = x ? 1 : 200;
	float x_px_size = b.width() / MATRIX_WIDTH;
	float y_px_size = b.height() / MATRIX_HEIGHT;

	for (size_t ym = 0; ym < MATRIX_HEIGHT; ++ym)
	{
		size_t tau = ym + TAU_MIN;
		size_t xm_now = this->beat_tracking.get_time() % tau;

		for (size_t xm = 0; xm < tau; ++xm)
		{
			float value = matrix[ym * MATRIX_WIDTH + xm] * scaling;
			float x0 = b.left + x_px_size * (float)xm;
			float y0 = b.top + y_px_size * (float)ym;
			float x1 = x0 + x_px_size;
			float y1 = y0 + y_px_size;

			// find the right color for this pixel
			Color col;
			if (
				c_current != nullptr &&
				tau == this->beat_tracking.get_current_tau() &&
				xm == this->beat_tracking.get_current_x()
			)
			{
				col = *c_current;
			}
			else if (
				c_new != nullptr &&
				tau == this->beat_tracking.get_new_tau() &&
				xm == this->beat_tracking.get_new_x()
			)
			{
				col = *c_new;
			}
			else if (xm == xm_now)
			{
				col = c_now;
			}
			else
			{
				col = c;
				col.a = value;
			}


			S2D_DrawQuad(
				x0, y0, COMMA_SPLIT_COLOR(col),
				x1, y0, COMMA_SPLIT_COLOR(col),
				x1, y1, COMMA_SPLIT_COLOR(col),
				x0, y1, COMMA_SPLIT_COLOR(col)
			);
		}
	}
}

void MyApp::render_x_matrix(Bounds b, Color c, Color c_now)
{
	// text
	SET_TEXT_COL(app.text_x_matrix, c);
	app.text_x_matrix->x = b.left;
	app.text_x_matrix->y = b.top;
	S2D_DrawText(app.text_x_matrix);

	// matrix
	b.top += 30;
	this->render_matrix(true, b, c, c_now, nullptr, nullptr);
}

void MyApp::render_y_matrix(Bounds b, Color c, Color c_now, Color c_current, Color c_new)
{
	// text
	SET_TEXT_COL(app.text_y_matrix, c);
	app.text_y_matrix->x = b.left;
	app.text_y_matrix->y = b.top;
	S2D_DrawText(app.text_y_matrix);

	// matrix
	b.top += 30;
	this->render_matrix(false, b, c, c_now, &c_current, &c_new);
}

void MyApp::input_thread_main()
{
	ShiftRegister input_samples(STFT_HOP_SIZE);
	float sample;

	while (!app.halt && (read(0, &sample, sizeof(sample)) == sizeof(sample)))
	{
		input_samples.push(sample);

		if (app.beat_tracking(sample))
		{
			float samples[input_samples.get_len()];
			input_samples.get_content(samples);

			app.input_samples_max.push(max(samples, input_samples.get_len()));
			app.input_samples_min.push(min(samples, input_samples.get_len()));
			app.odf_samples.push(app.beat_tracking.get_odf_sample());
			app.pp_odf_samples.push(app.beat_tracking.get_pp_odf_sample());
		}
	}
}

void MyApp::on_key(S2D_Event e)
{
	if (e.type == S2D_KEY_DOWN && strcmp(e.key, "Space") == 0)
	{
		app.beat_tracking.reset();
	}
}

void MyApp::render()
{
	float y = 0;

	app.render_audio_input(Bounds {0, y, WIDTH, y += 200}, C_GREEN);
	app.render_odf(Bounds {0, y, WIDTH, y += 200}, C_YELLOW, C_ORANGE);
	app.render_pp_odf(Bounds {0, y, WIDTH, y += 200}, C_RED);
	app.render_x_matrix(
		Bounds{
			0, y,
			(float)(MATRIX_WIDTH * MATRIX_PX_SIZE),
			(float)(y + MATRIX_HEIGHT * MATRIX_PX_SIZE + 30)
		},
		C_WHITE, C_WHITE
	);
	app.render_y_matrix(
		Bounds{
			(float)(MATRIX_WIDTH * MATRIX_PX_SIZE), y,
			(float)(MATRIX_WIDTH * MATRIX_PX_SIZE * 2),
			(float)(y += MATRIX_HEIGHT * MATRIX_PX_SIZE + 30)
		},
		C_WHITE, C_WHITE, C_GREEN, C_CYAN
	);
	app.render_info_box(Bounds {0, y, WIDTH - 200, y + 200}, C_GREEN, C_CYAN);
	app.render_beat_indicator(Bounds {WIDTH - 200, y, WIDTH, y+= 200}, C_RED);
	app.render_help(Bounds {0, y, WIDTH, y += 30}, C_WHITE);
}

void MyApp::render_grid_lines(Bounds b, Color c)
{
	float spacing = 1.0f / ODF_SAMPLE_INTERVAL;

	for (int i = 0;; ++i)
	{
		float x = b.right - (float)i * spacing;

		if (x < b.left)
		{
			break;
		}

		S2D_DrawLine(
			x, b.top, x, b.bottom, 1,
			COMMA_SPLIT_COLOR_4(c)
		);
	}
}

void MyApp::init()
{
	app.beat_tracking = _2011_PlRoSt();
	app.input_samples_min = ShiftRegister(WIDTH);
	app.input_samples_max = ShiftRegister(WIDTH);
	app.odf_samples = ShiftRegister(WIDTH);
	app.pp_odf_samples = ShiftRegister(WIDTH);
	app.text_audio_input = S2D_CreateText(
		FONT, "Audio Input", 20
	);
	assert(app.text_audio_input != nullptr);
	app.text_help = S2D_CreateText(
		FONT, "SPACE = set current tau and x to new tau and x", 20
	);
	assert(app.text_help != nullptr);
	app.text_odf = S2D_CreateText(
		FONT, "Onset Detection Function", 20
	);
	assert(app.text_odf != nullptr);
	app.text_pp_odf = S2D_CreateText(
		FONT, "Pre-Precessed Onset Detection Function", 20
	);
	assert(app.text_odf != nullptr);
	app.text_x_matrix = S2D_CreateText(
		FONT, "X(tau, phi)", 20
	);
	assert(app.text_x_matrix != nullptr);
	app.text_y_matrix = S2D_CreateText(
		FONT, "Y(tau, phi)", 20
	);
	assert(app.text_y_matrix != nullptr);
	app.halt = false;
	app.window = S2D_CreateWindow(
		TITLE, WIDTH, HEIGHT, nullptr, MyApp::render, 0
	);
	assert(app.window != nullptr);
	app.window->on_key = MyApp::on_key;
	app.input_thread = thread(MyApp::input_thread_main);
}

void MyApp::free()
{
	app.halt = true;
	app.input_thread.join();

	S2D_FreeText(app.text_audio_input);
	S2D_FreeText(app.text_help);
	S2D_FreeText(app.text_odf);
	S2D_FreeText(app.text_pp_odf);
	S2D_FreeText(app.text_x_matrix);
	S2D_FreeText(app.text_y_matrix);
	S2D_FreeWindow(app.window);
}

void MyApp::run()
{
	S2D_Show(app.window);
}


// ========== FUNCTIONS ========== //

int main()
{
	MyApp::init();
	MyApp::run();
	MyApp::free();
}