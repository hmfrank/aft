#include <simple2d.h>

// ========== CONSTANTS ========== //

// window title
const char *TITLE = "2011 Plumbley, Robertson, Stark - Real-Time Visual Beat Tracking Using a Comb Filter Matrix";

// font for text inside the window
const char *FONT = "res/roboto.ttf";

// window size in pixels
const int WIDTH = 1000;
const int HEIGHT = 300;


// ========== APP CLASS ========== //

class MyApp
{
	private:
		S2D_Window *window;

		static void render()
		{

		}

	public:
		static void init();

		static void free();

		static void run();
};

MyApp app;

void MyApp::init()
{
	app.window = S2D_CreateWindow(
		TITLE, WIDTH, HEIGHT, nullptr, MyApp::render, 0
	);
}

void MyApp::free()
{
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