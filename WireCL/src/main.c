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
//#ifndef __MINGW32__
#include <CL/cl.h>
//#endif
#include <pthread.h>

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
double lfms = 0.;

void displayCallback() {
	double cms = (double) ts.tv_sec * 1000. + (double) ts.tv_nsec / 1000000.;
	if (cms - lfms < (1000. / FRAMELIMIT)) {
		double usd = (cms - lfms);
		double ttnf = (1000. / FRAMELIMIT) - usd;
		//printf("ttnf %f %f\n", usd, ttnf);
		if (ttnf > 0.) {
			usleep(ttnf * 1000.);
		}
	}
	lfms = cms;
	frames++;
	glfwGetFramebufferSize(window, &width, &height);
	int sf = 1;
	csf = sf;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	double ms2 = (double) ts.tv_sec * 1000. + (double) ts.tv_nsec / 1000000.;
	while (ms2 > lt + 50.) {
		lt += 50.;
		//tick();
	}
	if (ms2 > lf + 1000.) {
		double t = ms2 - lf;
		lf = ms2;
		printf("FPS: %f Generation: %i GPS: %i\n", (float) frames / (t / 1000.), world->generation, world->gps);
		world->gps = 0;
		frames = 0;
	}

	//struct timespec ts2;
	//clock_gettime(CLOCK_MONOTONIC, &ts2);
	//printf("tick-time: %f\n", ((double) ts2.tv_sec * 1000. + (double) ts2.tv_nsec / 1000000.) - ms2);
	float partialTick = ((ms2 - lt) / 50.);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, width, height);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0., width * zoom, height * zoom, 0., 1000., 3000.);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0, 0, 0, 1);
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

void updateThread(void* arg) {
	while (1) {
		tick();
	}
	pthread_cancel (pthread_this());}

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
	else if (action == GLFW_RELEASE) mouseButton = -1;
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
			camX = world->width * 16. / 2.;
			camY = world->height * 16. / 2.;
		}
	}
	zoom = 1.;
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
	if (ecwd != NULL) {
		ecwd++;
		ecwd[0] = 0;
		chdir(ncwd);
	}
	printf("Loading... [FROM=%s]\n", strlen(INSTALLDIR) == 0 ? ncwd : INSTALLDIR);
//#ifndef __MINGW32__
#ifdef __MINGW32__
	char cwd2[1024];
	getcwd(cwd2, 1024);
	char path[4096];
	snprintf(path, 4096, "%s\\assets\\wireworld.cl", cwd2);
	int sfd = open(path, O_RDONLY);
#else
	int sfd = open("./assets/wireworld.cl", O_RDONLY);
#endif
	if (sfd < 0) {
		char cwd[1024];
		getcwd(cwd, 1024);
		printf("Couldn't open wireworld.cl! <%s> <%s>\n", cwd, strerror(errno));
		return 1;
	}
	char* src = malloc(4097);
	size_t tc = 4096;
	size_t r = 0;
	ssize_t x = 0;
	while ((x = read(sfd, src + r, tc - r)) > 0) {
		r += x;
		src[r] = 0;
		if (tc - r < 2048) {
			tc += 4096;
			src = realloc(src, tc + 1);
		}
	}
	close(sfd);
	cl_platform_id platform_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_device_id device_id = NULL;
	cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &ret_num_devices); // if u dont have a GPU, change to CPU (please no push)
	wire_context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
	wire_command_queue = clCreateCommandQueue(wire_context, device_id, 0, &ret);
	wire_program = clCreateProgramWithSource(wire_context, 1, (const char **) &src, (const size_t *) &r, &ret);
	ret = clBuildProgram(wire_program, 1, &device_id, NULL, NULL, NULL);
	char bl[65536];
	clGetProgramBuildInfo(wire_program, device_id, CL_PROGRAM_BUILD_LOG, 65536, bl, NULL);
	printf("Build Log: %s\n", bl);
//#endif
	pthread_t gpt;
	pthread_create(&gpt, NULL, updateThread, NULL);
	width = 800;
	height = 600;
	if (!glfwInit()) return -1;
	glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
	glfwWindowHint(GLFW_SAMPLES, 4); // antialiasing
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
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
	//glEnable (GL_DEPTH_TEST);
	//glEnable (GL_TEXTURE_2D);
	glEnable (GL_CULL_FACE);
	//glAlphaFunc(GL_GREATER, 0.1);
	//glEnable (GL_ALPHA_TEST);
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
