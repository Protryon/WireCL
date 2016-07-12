#include <stdio.h>
#include <stdlib.h>
#ifdef __MINGW32__
#define GLEW_STATIC
#include <GL/glew.h>
#endif
#include "gui.h"
#include "globals.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>
#include <pthread.h>
#include <time.h>
#include <png.h>
#include <math.h>
#include <errno.h>
#include "render.h"
#include <unistd.h>
#include <fcntl.h>
#include "world.h"

int fr = 30;
int rr = 0;
int b1d = 0;
float chx = 0.;
float chy = 0.;

void tick() {
	gui_tick();
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	gui_keyboardCallback(key, scancode, action, mods);
}

void textCallback(GLFWwindow* window, unsigned int codepoint) {
	gui_textCallback(codepoint);
}

struct timespec ts;
double lt;
double lf;
int frames;

void displayCallback() {
	frames++;
	glfwGetFramebufferSize(window, &width, &height);
	int v5 = 1000;
	int sf = 1;
	swidth = width;
	sheight = height;
	while (sf < v5 && swidth / (sf + 1) >= 320 && sheight / (sf + 1) >= 240) {
		sf++;
	}
	swidth = (int) ceil(swidth / sf);
	sheight = (int) ceil(sheight / sf);
	csf = sf;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	double ms2 = (double) ts.tv_sec * 1000. + (double) ts.tv_nsec / 1000000.;
	while (ms2 > lt + 50.) {
		lt += 50.;
		tick();
	}
	if (ms2 > lf + 1000.) {
		double t = ms2 - lf;
		lf = ms2;
		printf("FPS: %f\n", (float) frames / (t / 1000.));
		frames = 0;
	}

	//struct timespec ts2;
	//clock_gettime(CLOCK_MONOTONIC, &ts2);
	//printf("tick-time: %f\n", ((double) ts2.tv_sec * 1000. + (double) ts2.tv_nsec / 1000000.) - ms2);
	float partialTick = ((ms2 - lt) / 50.);
	glEnable (GL_TEXTURE_2D);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, width, height);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0., swidth, sheight, 0., 1000., 3000.);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0., 0., -2000.);
	drawGUI(partialTick);
	size_t error = glGetError();
	if (error) {
		char* es;
		if (error == GL_INVALID_ENUM) {
			es = "GL_INVALID_ENUM";
		} else if (error == GL_INVALID_VALUE) {
			es = "GL_INVALID_VALUE";
		} else if (error == GL_INVALID_OPERATION) {
			es = "GL_INVALID_OPERATION";
#ifdef GL_INVALID_FRAMEBUFFER_OPERATION
		} else if (error == GL_INVALID_FRAMEBUFFER_OPERATION) {
			es = "GL_INVALID_FRAMEBUFFER_OPERATION";
#endif
		} else if (error == GL_STACK_OVERFLOW) {
			es = "GL_STACK_OVERFLOW";
		} else if (error == GL_STACK_UNDERFLOW) {
			es = "GL_STACK_UNDERFLOW";
		} else if (error == GL_OUT_OF_MEMORY) {
			es = "GL_OUT_OF_MEMORY";
		} else es = "UNKNOWN";
		printf("GLError: %s\n", es);
	}
	glFlush();
	mouseButton = -1;
	//clock_gettime(CLOCK_MONOTONIC, &ts2);
	//printf("render-time: %f\n", ((double) ts2.tv_sec * 1000. + (double) ts2.tv_nsec / 1000000.) - ms2);
}

void mouseMotionCallback(GLFWwindow* window, double x, double y) {
	gui_mouseMotionCallback(x, y);
	mouseX = x / (csf == 0 ? 1 : csf);
	mouseY = y / (csf == 0 ? 1 : csf);
}

void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == 3 || button == 4) {
		//if (state == GLUT_UP) return;
		//if (button == 3) {
		//	heldItem++;
		//	if (heldItem == 9) heldItem = 0;
		//} else {
		//	heldItem--;
		//	if (heldItem == -1) heldItem = 8;
		//}
	}
	if (action == GLFW_PRESS) mouseButton = button;
}

void scrollCallback(GLFWwindow* window, double x, double y) {
	gui_scrollCallback(x, y);
}

void error_callback(int error, const char* description) {
	printf("GLFW error: %s\n", description);
}

void cursorEnterCallback(GLFWwindow* window, int entered) {
	hasMouse = entered;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: WireCL <file>\n");
		return 0;
	}
	for (int i = 1; i < argc; i++) {
		if (i == argc - 1) {
			world = newWorld();
			loadWorldText(argv[i], world);
		}
	}
//#ifdef __MINGW32__
//	WORD versionWanted = MAKEWORD(1, 1);
//	WSADATA wsaData;
//	WSAStartup(versionWanted, &wsaData);
//#endif
	char ncwd[strlen(argv[0]) + 1];
	memcpy(ncwd, argv[0], strlen(argv[0]) + 1);
	char* ecwd =
#ifdef __MINGW32__
			strrchr(ncwd, '\\');
#else
			strrchr(ncwd, '/');
#endif
	ecwd++;
	ecwd[0] = 0;
	chdir(ncwd);
	printf("Loading... [FROM=%s]\n", strlen(INSTALLDIR) == 0 ? ncwd : INSTALLDIR);
	width = 800;
	height = 600;
	if (!glfwInit()) return -1;
	glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
	glfwWindowHint(GLFW_SAMPLES, 4); // antialiasing
	glfwSetErrorCallback(error_callback);
	window = glfwCreateWindow(800, 600, "WireCL", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent (window);
#ifdef __MINGW32__
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if(err != GLEW_OK) {
		printf("GLEW Init error: %s\n", glewGetErrorString(err));
		glfwTerminate();
		return -1;
	}
	if(!glewIsSupported("GL_VERSION_2_1") || !glewIsSupported("GL_ARB_vertex_program")) {
		printf("OpenGL version 2.1+ or GL_ARB_vertex_program not satisfied.\n");
		glfwTerminate();
		return -1;
	}
#endif
	printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
	//glEnable (GL_MULTISAMPLE);
	//glEnable (GL_MULTISAMPLE_ARB);
	glEnable (GL_DEPTH_TEST);
	glEnable (GL_TEXTURE_2D);
	glEnable (GL_CULL_FACE);
	glAlphaFunc(GL_GREATER, 0.1);
	glEnable (GL_ALPHA_TEST);
	loadGUI();
	clock_gettime(CLOCK_MONOTONIC, &ts);
	lt = (double) ts.tv_sec * 1000. + (double) ts.tv_nsec / 1000000.;
	lf = lt;
	glfwSetKeyCallback(window, keyboardCallback);
	glfwSetCursorEnterCallback(window, cursorEnterCallback);
	glfwSetCharCallback(window, textCallback);
	glfwSetCursorPosCallback(window, mouseMotionCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);
	glfwSetScrollCallback(window, scrollCallback);
	printf("Loaded.\n");
	while (!glfwWindowShouldClose(window)) {
		displayCallback();
		glfwSwapBuffers(window);
		glfwSwapInterval(1);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
//glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB | GLUT_MULTISAMPLE);

//glutKeyboardFunc(keyboardCallback);
//glutKeyboardUpFunc(keyboardUpCallback);
//glutSpecialFunc(keyboardSpecCallback);
//glutDisplayFunc(displayCallback);
//glutMotionFunc(mouseMotionCallback);
//glutMouseFunc(mouseCallback);
//glutPassiveMotionFunc(mouseMotionCallback);
//glutSetKeyRepeat (GLUT_KEY_REPEAT_OFF);
}
