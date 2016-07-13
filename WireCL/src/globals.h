/*
 * gloabls.h
 *
 *  Created on: Mar 5, 2016
 *      Author: root
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <GLFW/glfw3.h>
//#ifndef __MINGW32__
#include <CL/cl.h>
//#endif

#define FRAMELIMIT 60.

int width;
int height;
int mouseX;
int mouseY;
int mouseButton;
int csf;
int hasMouse;
GLFWwindow* window;
struct world* world;
float zoom;
float camX;
float camY;
int paused;
int reflush;
//#ifndef __MINGW32__
cl_program wire_program;
cl_command_queue wire_command_queue;
cl_context wire_context;
//#endif
#define PI 3.141592653589793

#ifdef __MINGW32__
#define INSTALLDIR ""
#else
#define INSTALLDIR ""
#endif

#endif /* GLOBALS_H_ */
