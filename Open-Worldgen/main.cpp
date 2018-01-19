#include <iostream>
#include <GL/glew.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include "GlWindow.h"
using std::cout;
using std::endl;
int main(int argc, char **argv) {
	// GUI variables (mostly for position and size)
	int winWidth = 800;
	int winHeight = 800;
	int winXPos = winWidth / 2;
	int winYPos = winHeight / 2;

	// Set up our graphics window
	GlWindow *mainWindow = new GlWindow(0, 0, winWidth, winHeight, "");
	mainWindow->redraw();
	mainWindow->label("Open Worldgen v1.0");
	mainWindow->end();

	// Make the window visible
	mainWindow->show(argc, argv);
	return Fl::run();
}