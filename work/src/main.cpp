//---------------------------------------------------------------------------
//
// Copyright (c) 2016 Taehyun Rhee, Joshua Scott, Ben Allen
//
// This software is provided 'as-is' for assignment of COMP308 in ECS,
// Victoria University of Wellington, without any express or implied warranty. 
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// The contents of this file may not be copied or duplicated in any form
// without the prior permission of its owner.
//
//----------------------------------------------------------------------------

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <stdexcept>

#include "cgra_math.hpp"
#include "opengl.hpp"
#include "skeleton.hpp"
#include "simple_gui.hpp"

using namespace std;
using namespace cgra;

// Window
//
GLFWwindow* g_window;


// Projection values
// 
float g_fovy = 20.0;
float g_znear = 0.001;
float g_zfar = 1000.0;


// Mouse controlled Camera values
//
bool g_leftMouseDown = false;
vec2 g_mousePosition;
float g_pitch = 0;
float g_yaw = 0;
float g_zoom = 1.0;
float g_pan_y = 0;
float g_pan_x = 0;

unsigned char pixel[3];


// Geometry loader and drawer
//
Skeleton *g_skeleton = nullptr;

/*
Taken from 3dbuzzz
*/
vec3 screenToWorld(int x, int y) {
	GLdouble
		posX1,
		posY1,
		posZ1,
		posX2,
		posY2,
		posZ2,
		modelview[16],
		projection[16];
	GLint viewport[4];
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Create ray
	gluUnProject(x, viewport[1] + viewport[3] - y, 0, modelview, projection, viewport, &posX1, &posY1, &posZ1);  // Near plane
	gluUnProject(x, viewport[1] + viewport[3] - y, 1, modelview, projection, viewport, &posX2, &posY2, &posZ2);  // Far plane

	GLfloat
		t = (posZ1 - 0) / (posZ1 - posZ2);  // - 0 Since our Z == 0.. For easier editing later

											// so here are the desired (x, y) coordinates
	GLfloat
		fX = posX1 + (posX2 - posX1) * t,
		fY = posY1 + (posY2 - posY1) * t;

	return vec3(fX, fY, 0);
}

void readPixel() {
	GLint viewport[3];
	glGetIntegerv(GL_VIEWPORT, viewport);


	glReadPixels(g_mousePosition.x, viewport[3] - g_mousePosition.y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
	//cout << "[ " << (int)pixel[0] << " ]" << endl;
}

// Mouse Button callback
// Called for mouse movement event on since the last glfwPollEvents
//
void cursorPosCallback(GLFWwindow* win, double xpos, double ypos) {
	// cout << "Mouse Movement Callback :: xpos=" << xpos << "ypos=" << ypos << endl;

	if (g_leftMouseDown) {
		g_yaw -= g_mousePosition.x - xpos;
		g_pitch -= g_mousePosition.y - ypos;

	}
	g_mousePosition = vec2(xpos, ypos);
}


// Mouse Button callback
// Called for mouse button event on since the last glfwPollEvents
//
void mouseButtonCallback(GLFWwindow *win, int button, int action, int mods) {
	cout << "Mouse Button Callback :: button=" << button << "action=" << action << "mods=" << mods << endl;
	// send messages to the GUI manually
	SimpleGUI::mouseButtonCallback(win, button, action, mods);

	if (button == GLFW_MOUSE_BUTTON_LEFT&&action == 0) {
		vec3 wPos = screenToWorld(g_mousePosition.x, g_mousePosition.y);

		g_skeleton->setSelected(
			pixel[0] +
			pixel[1] * 256 +
			pixel[2] * 256 * 256);
	}

	g_leftMouseDown = (action == GLFW_PRESS);

}



// Scroll callback
// Called for scroll event on since the last glfwPollEvents
//
void scrollCallback(GLFWwindow *win, double xoffset, double yoffset) {
	// cout << "Scroll Callback :: xoffset=" << xoffset << "yoffset=" << yoffset << endl;
	g_zoom -= yoffset * g_zoom * 0.2;
}


//-------------------------------------------------------------
// [Assignment 2] :
// Modify the keyCallback function and additional files,
// to make your priman pose when the 'p' key is pressed.
//-------------------------------------------------------------

// Keyboard callback
// Called for every key event on since the last glfwPollEvents
//
void keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) {
	// cout << "Key Callback :: key=" << key << "scancode=" << scancode
	// 	<< "action=" << action << "mods=" << mods << endl;
	// YOUR CODE GOES HERE
	// ...
	cout << key << endl;
	if (action == 0) {
		switch (key) {
		case 87:g_pan_y += 0.05;
			break;
		case 83:g_pan_y += -0.05;;
			break;
		case 65:g_pan_x += -0.05;
			break;
		case 68:g_pan_x += 0.05;
			break;
		}
	}
}


// Character callback
// Called for every character input event on since the last glfwPollEvents
//
void charCallback(GLFWwindow *win, unsigned int c) {
	// cout << "Char Callback :: c=" << char(c) << endl;
	// Not needed for this assignment, but useful to have later on
}


// Sets up where and what the light is
// 
void setupLight() {
	// No transform for the light
	// makes it move realitive to camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	vec4 direction(0.0, 0.0, 1.0, 0.0);
	vec4 diffuse(0.7, 0.7, 0.7, 1.0);
	vec4 ambient(0.2, 0.2, 0.2, 1.0);

	glLightfv(GL_LIGHT0, GL_POSITION, direction.dataPointer());
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse.dataPointer());
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient.dataPointer());

	glEnable(GL_LIGHT0);
}


// Sets up where the camera is in the scene
// 
void setupCamera(int width, int height) {
	// Set up the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(g_fovy, width / float(height), g_znear, g_zfar);

	// Set up the view part of the model view matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(g_pan_x, g_pan_y, -5 * g_zoom);
	glRotatef(g_pitch, 1, 0, 0);
	glRotatef(g_yaw, 0, 1, 0);
}


// Render one frame to the current window given width and height
//
void render(int width, int height) {

	// Set viewport to be the whole window
	glViewport(0, 0, width, height);

	// Grey/Blueish background
	glClearColor(0.3f, 0.3f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Enable flags for normal rendering
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	// Render geometry
	g_skeleton->renderSkeleton();

	// Disable flags for cleanup (optional)
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_NORMALIZE);
	glDisable(GL_COLOR_MATERIAL);
}


//-------------------------------------------------------------
// [Assignment 2] :
// Modify the renderGUI function to implement a basic
// player when using AMC data for your skeleton.
//
// Play 		- Play AMC data at 1 step per frame 
// Pause 		- Stop playing but remain at current frame 
// Stop 		- Stop playing and go back to first frame
// Rewind 		- Play AMC data at -1 steps per frame
// Fast Forward - Play AMC data at 2 times current speed
//
// There is no need to learn or use SimpleGUI/IMGUI;
// simply fill out each part of each "if statment"
//-------------------------------------------------------------

// Renders IMGUI ontop of your render output
//
void renderGUI() {
	// Start registering GUI components
	SimpleGUI::newFrame();

	if (ImGui::IsMouseClicked(1))
		ImGui::OpenPopup("Player");

	if (ImGui::BeginPopup("Player")) {
		if (ImGui::Selectable("Play")) {
			g_skeleton->rate = 1;
		}

		if (ImGui::Selectable("Pause")) {
			g_skeleton->rate = 0;
		}

		if (ImGui::Selectable("Stop")) {
			// YOUR CODE GOES HERE
			// ...

		}

		if (ImGui::Selectable("Rewind")) {
			if (g_skeleton->rate == 1) {
				g_skeleton->rate = -1;
			}
			if (g_skeleton->rate == -1) {
				g_skeleton->rate = 1;
			}
		}

		if (ImGui::Selectable("Fast Forward")) {
			if (g_skeleton->rate == 1) {
				g_skeleton->rate = 2;
			}
			else if (g_skeleton->rate == -1) {
				g_skeleton->rate = -2;
			}
			else if (g_skeleton->rate == 2 || g_skeleton->rate == -2) {
				g_skeleton->rate == 1;
			}
		}

		ImGui::EndPopup();
	}

	// Flush components and render
	SimpleGUI::render();
}



// Forward decleration for cleanliness (Ignore)
void APIENTRY debugCallbackARB(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar*, GLvoid*);


//Main program
// 
int main(int argc, char **argv) {

	// Check argument list
	if (argc < 2) {
		cout << "ASF filename expected, eg:" << endl << "    ./a2 priman.asf" << endl;
		abort(); // Unrecoverable error
	}
	else if (argc > 3) {
		cout << "Too many arguments, expected only ASF and AMC filenames, eg:" << endl;
		cout << "    ./a2 priman.asf priman.amc" << endl;
		abort(); // Unrecoverable error
	}



	// Initialize the GLFW library
	if (!glfwInit()) {
		cerr << "Error: Could not initialize GLFW" << endl;
		abort(); // Unrecoverable error
	}

	// Get the version for GLFW for later
	int glfwMajor, glfwMinor, glfwRevision;
	glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);

	// Create a windowed mode window and its OpenGL context
	g_window = glfwCreateWindow(640, 480, "Hello World", nullptr, nullptr);
	if (!g_window) {
		cerr << "Error: Could not create GLFW window" << endl;
		abort(); // Unrecoverable error
	}

	// Make the g_window's context is current.
	// If we have multiple windows we will need to switch contexts
	glfwMakeContextCurrent(g_window);



	// Initialize GLEW
	// must be done after making a GL context current (glfwMakeContextCurrent in this case)
	glewExperimental = GL_TRUE; // required for full GLEW functionality for OpenGL 3.0+
	GLenum err = glewInit();
	if (GLEW_OK != err) { // Problem: glewInit failed, something is seriously wrong.
		cerr << "Error: " << glewGetErrorString(err) << endl;
		abort(); // Unrecoverable error
	}



	// Print out our OpenGL verisions
	cout << "Using OpenGL " << glGetString(GL_VERSION) << endl;
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;
	cout << "Using GLFW " << glfwMajor << "." << glfwMinor << "." << glfwRevision << endl;



	// Attach input callbacks to g_window
	glfwSetCursorPosCallback(g_window, cursorPosCallback);
	glfwSetMouseButtonCallback(g_window, mouseButtonCallback);
	glfwSetScrollCallback(g_window, scrollCallback);
	glfwSetKeyCallback(g_window, keyCallback);
	glfwSetCharCallback(g_window, charCallback);



	// Enable GL_ARB_debug_output if available. Not nessesary, just helpful
	if (glfwExtensionSupported("GL_ARB_debug_output")) {
		// This allows the error location to be determined from a stacktrace
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		// Set the up callback
		glDebugMessageCallbackARB(debugCallbackARB, nullptr);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
		cout << "GL_ARB_debug_output callback installed" << endl;
	}
	else {
		cout << "GL_ARB_debug_output not available. No worries." << endl;
	}



	// Initialize IMGUI
	// Second argument is true if we dont need to use GLFW bindings for input
	// if set to false we must manually call the SimpleGUI callbacks when we
	// process the input.
	if (!SimpleGUI::init(g_window, false)) {
		cerr << "Error: Could not initialize IMGUI" << endl;
		abort();
	}



	// Initialize our scene
	g_skeleton = new Skeleton(argv[1]);
	if (argc == 3) {
		g_skeleton->readAMC(argv[2]);
	}




	// Loop until the user closes the window
	while (!glfwWindowShouldClose(g_window)) {

		// Make sure we draw to the WHOLE window
		int width, height;
		glfwGetFramebufferSize(g_window, &width, &height);

		glDisable(GL_LIGHTING);

		// Main Render
		g_skeleton->color = false;

		render(width, height);

		readPixel();

		// Setup light
		setupLight();

		// Setup camera
		setupCamera(width, height);

		glEnable(GL_LIGHTING);

		g_skeleton->color = true;
		render(width, height);

		// Render GUI on top
		renderGUI();



		// Swap front and back buffers
		glfwSwapBuffers(g_window);

		// Poll for and process events
		glfwPollEvents();
	}

	glfwTerminate();
}






//-------------------------------------------------------------
// Fancy debug stuff
//-------------------------------------------------------------

// function to translate source to string
string getStringForSource(GLenum source) {

	switch (source) {
	case GL_DEBUG_SOURCE_API:
		return("API");
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		return("Window System");
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		return("Shader Compiler");
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		return("Third Party");
	case GL_DEBUG_SOURCE_APPLICATION:
		return("Application");
	case GL_DEBUG_SOURCE_OTHER:
		return("Other");
	default:
		return("n/a");
	}
}

// function to translate severity to string
string getStringForSeverity(GLenum severity) {

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		return("HIGH!");
	case GL_DEBUG_SEVERITY_MEDIUM:
		return("Medium");
	case GL_DEBUG_SEVERITY_LOW:
		return("Low");
	default:
		return("n/a");
	}
}

// function to translate type to string
string getStringForType(GLenum type) {
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		return("Error");
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		return("Deprecated Behaviour");
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		return("Undefined Behaviour");
	case GL_DEBUG_TYPE_PORTABILITY:
		return("Portability Issue");
	case GL_DEBUG_TYPE_PERFORMANCE:
		return("Performance Issue");
	case GL_DEBUG_TYPE_OTHER:
		return("Other");
	default:
		return("n/a");
	}
}

// actually define the function
void APIENTRY debugCallbackARB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei, const GLchar* message, GLvoid*) {
	// if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) return;

	cerr << endl; // extra space

	cerr << "Type: " <<
		getStringForType(type) << "; Source: " <<
		getStringForSource(source) << "; ID: " << id << "; Severity: " <<
		getStringForSeverity(severity) << endl;

	cerr << message << endl;

	if (type == GL_DEBUG_TYPE_ERROR_ARB) throw runtime_error("");
}