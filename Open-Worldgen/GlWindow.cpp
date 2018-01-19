#include "GlWindow.h"

/*********************************************************************
* Draw Function Override                                             *
* - Overrides default GlWindow draw function and draws the center of *
*   gravity, our die, and the three axes                             *
**********************************************************************/
void GlWindow::draw() {
	// Set up our viewport
	if (!valid()) {
		glViewport(0, 0, GlWindow::windowWidth, GlWindow::windowHeight);
		glClearColor(.0f, .0f, .0f, 1);
		glEnable(GL_DEPTH_TEST);
		valid();
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);      // clear the color and depth buffer

															 // View Transformations
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1, 1, -1, 1, 1, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	this->setLook(this->eye, this->center, this->up);

	GlWindow::drawSector(0.0, 0.0, 0.0);
}

/*********************************************************************
* GlWindow Handle Override                                           *
* - Overrides default GlWindow handle, determines how to handle user *
*   input (basic event handling)                                     *
**********************************************************************/
int GlWindow::handle(int event) {
	switch (event) {
	case FL_PUSH:
		return 1;
	case FL_ENTER:
		this->take_focus();
		return 1;
	case FL_DRAG:
		// Right mouse button
		if ((Fl::event_state() & FL_BUTTON3) || (Fl::event_state() & FL_BUTTON2)) {
			// Holds the current Cursor position
			vec2		cPos = vec2(Fl::event_x(), Fl::event_y());
						dV = cPos - lPos;			// use the current position and the last position to calculate cursor velocity 
			float		dVL = glm::length(dV);		// Holds the length of the delta vector
			int			maxX = this->windowWidth;	// Holds the maximum allowed X position
			int			maxY = this->windowHeight;	// Holds the maximum allowed Y position
			if (cPos.x >= 0 && (cPos.x <= maxX) && cPos.y >= 0 && (cPos.y <= maxY)) {
				// Rotate the die and then redraw to update view
				this->rotateEye(up, rotSpeed * (dV.x / dVL));
				this->rotateEye(right, rotSpeed * (dV.y / dVL));
				this->setLook(eye, center, up);
				this->redraw();
			}
			lPos = cPos;
		}
		if ((Fl::event_state() & FL_BUTTON1)) {
			// TODO: Something Here
			vec2		cPos = vec2(Fl::event_x(), Fl::event_y());
						dV	 = cPos - lPos;			// use the current position and the last position to calculate cursor velocity 
			float		dVL	 = glm::length(dV);		// Holds the length of the delta vector
			this->eye -= zoomSpeed * look * (dV.y / dVL);
			this->redraw();
			lPos = cPos;
		}
	case FL_RELEASE:
		return 1;
	case FL_FOCUS:
		return 1;
	case FL_UNFOCUS:
		return 1;
	case FL_KEYBOARD:
		// UP -- Rotate die up
		if (Fl::event_key() == FL_Up) {
			rotateEye(right, panSpeed);
			redraw();
		}
		// DOWN -- Rotate die down
		if (Fl::event_key() == FL_Down) {
			rotateEye(right, -panSpeed);
			redraw();
		}
		// LEFT -- Rotate die left
		if (Fl::event_key() == FL_Left) {
			rotateEye(up, panSpeed);
			redraw();
		}
		// RIGHT -- Rotate die right
		if (Fl::event_key() == FL_Right) {
			rotateEye(up, -panSpeed);
			redraw();
		}
		return 1;
	case FL_KEYUP:
		return 0;
	default:
		// pass other events to the base class...
		return Fl_Gl_Window::handle(event);
	}
	return 0;
}

/*********************************************************************
* Set Look Function                                                  *
* - Uses the eye, the center and the up vectors for the camera to    *
*   set where the camera is looking                                  *
* - Helper function which is used by the GlWindow's handle           *
**********************************************************************/
void GlWindow::setLook(vec3 ePos, vec3 cPos, vec3 uPos) {
	gluLookAt(ePos.x, ePos.y, ePos.z, cPos.x, cPos.y, cPos.z, uPos.x, uPos.y, uPos.z);
}

/*********************************************************************
* Rotate Eye Function                                                *
* - Uses the camera's right, up, look, and eye vectors to create a   *
*   rotation matrix and then rotate the eye around the axis of       *
*   rotation by the given angle                                      *
* - Helper function which is used by the GlWindow's handle           *
**********************************************************************/
void GlWindow::rotateEye(vec3 AOR, float angle) {
	glm::mat4x4 V = { right.x, up.x, look.x, eye.x,
		right.y, up.y, look.y, eye.y,
		right.z, up.z, look.z, eye.z,
		0.0f   ,  0.0f,  0.0f  ,  1.0f };

	V = glm::rotate(V, angle, AOR);
	right = vec3(V[0][0], V[1][0], V[2][0]);
	up = vec3(V[0][1], V[1][1], V[2][1]);
	look = vec3(V[0][2], V[1][2], V[2][2]);
	eye = vec3(V[0][3], V[1][3], V[2][3]);
}

/*********************************************************************
* Draw Dice Function                                                 *
* - Responsible for drawing and texturing the dice model in the      *
*   GLWINDOW                                                         *
* - Helper function for the drawScene function                       *
**********************************************************************/
void GlWindow::drawSector(float x, float y, float z) {
	// Load textures for the cube (apparently we have to do this separately because the alias for it is messed up)
	/*
	// TODO: TEXTURE LOADING HERE
	FL_JPEG_Image grass("./assets/textures/grass.jpg");
	...
	*/

	// Move position back to the origin
	glTranslatef(x, -y, -z);

	// Start drawing our die
	/*
	// TODO: SET TEXTURE FOR IMAGE HERE, EVENTUALLY NEED TO READ IN TEXTURE FROM SECTOR OBJECT
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, grass.w(), grass.h(), 0, GL_RGB, GL_UNSIGNED_BYTE, grass.data()[0]);
	*/

	glBegin(GL_QUADS);
	// front
	glColor4f(1, 0, 0, .4);
	glTexCoord2f(1.0, 0.0); glVertex3f(secSize, secSize, secSize); // Top Right Of The Texture and Quad
	glTexCoord2f(0.0, 0.0); glVertex3f(-secSize, secSize, secSize); // Top Left Of The Texture and Quad
	glTexCoord2f(0.0, 1.0); glVertex3f(-secSize, -secSize, secSize); // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0, 1.0); glVertex3f(secSize, -secSize, secSize); // Bottom Right Of The Texture and Quad
	glEnd();

	/*
	// TODO: SET TEXTURE FOR IMAGE HERE, EVENTUALLY NEED TO READ IN TEXTURE FROM SECTOR OBJECT
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, grass.w(), grass.h(), 0, GL_RGB, GL_UNSIGNED_BYTE, grass.data()[0]);
	*/

	glBegin(GL_QUADS);
	// back
	glColor4f(1, 0, 0, .4);
	glTexCoord2f(1.0, 0.0); glVertex3f(-secSize, secSize, -secSize);
	glTexCoord2f(0.0, 0.0); glVertex3f(secSize, secSize, -secSize);
	glTexCoord2f(0.0, 1.0); glVertex3f(secSize, -secSize, -secSize);
	glTexCoord2f(1.0, 1.0); glVertex3f(-secSize, -secSize, -secSize);
	glEnd();

	/*
	// TODO: SET TEXTURE FOR IMAGE HERE, EVENTUALLY NEED TO READ IN TEXTURE FROM SECTOR OBJECT
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, grass.w(), grass.h(), 0, GL_RGB, GL_UNSIGNED_BYTE, grass.data()[0]);
	*/

	glBegin(GL_QUADS);
	// top
	glColor4f(0, 1, 0, .4);
	glTexCoord2f(1.0, 0.0); glVertex3f(-secSize, secSize, -secSize);
	glTexCoord2f(0.0, 0.0); glVertex3f(-secSize, secSize, secSize);
	glTexCoord2f(0.0, 1.0); glVertex3f(secSize, secSize, secSize);
	glTexCoord2f(1.0, 1.0); glVertex3f(secSize, secSize, -secSize);
	glEnd();

	/*
	// TODO: SET TEXTURE FOR IMAGE HERE, EVENTUALLY NEED TO READ IN TEXTURE FROM SECTOR OBJECT
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, grass.w(), grass.h(), 0, GL_RGB, GL_UNSIGNED_BYTE, grass.data()[0]);
	*/

	glBegin(GL_QUADS);
	// bottom
	glColor4f(0, 1, 0, .4);
	glTexCoord2f(1.0, 0.0); glVertex3f(-secSize, -secSize, -secSize);
	glTexCoord2f(0.0, 0.0); glVertex3f(secSize, -secSize, -secSize);
	glTexCoord2f(0.0, 1.0); glVertex3f(secSize, -secSize, secSize);
	glTexCoord2f(1.0, 1.0); glVertex3f(-secSize, -secSize, secSize);
	glEnd();

	/*
	// TODO: SET TEXTURE FOR IMAGE HERE, EVENTUALLY NEED TO READ IN TEXTURE FROM SECTOR OBJECT
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, grass.w(), grass.h(), 0, GL_RGB, GL_UNSIGNED_BYTE, grass.data()[0]);
	*/

	glBegin(GL_QUADS);
	// left
	glColor4f(0, 0, 1, .4);
	glTexCoord2f(1.0, 0.0); glVertex3f(-secSize, secSize, -secSize);
	glTexCoord2f(0.0, 0.0); glVertex3f(-secSize, -secSize, -secSize);
	glTexCoord2f(0.0, 1.0); glVertex3f(-secSize, -secSize, secSize);
	glTexCoord2f(1.0, 1.0); glVertex3f(-secSize, secSize, secSize);
	glEnd();

	/*
	// TODO: SET TEXTURE FOR IMAGE HERE, EVENTUALLY NEED TO READ IN TEXTURE FROM SECTOR OBJECT
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, grass.w(), grass.h(), 0, GL_RGB, GL_UNSIGNED_BYTE, grass.data()[0]);
	*/

	glBegin(GL_QUADS);
	// right
	glColor4f(0, 0, 1, .4);
	glTexCoord2f(1.0, 0.0); glVertex3f(secSize, secSize, secSize);
	glTexCoord2f(0.0, 0.0); glVertex3f(secSize, -secSize, secSize);
	glTexCoord2f(0.0, 1.0); glVertex3f(secSize, -secSize, -secSize);
	glTexCoord2f(1.0, 1.0); glVertex3f(secSize, secSize, -secSize);
	glEnd();
}

/*********************************************************************
* Reset View Function                                                *
* - Responsible for resetting the camera vectors to their original   *
*   default positions                                                *
* - Restores view to front facing                                    *
**********************************************************************/
void GlWindow::resetView() {
	eye = vec3(0.0f, 0.0f, 3.0f);
	center = vec3(0.0f, 0.0f, 0.0f);
	up = vec3(0.0f, 1.0f, 0.0f);
	look = vec3(0.0f, 0.0f, -1.0f);
	right = vec3(1.0, 0.0f, 0.0f);
	this->setLook(eye, center, up);
	this->redraw();
}