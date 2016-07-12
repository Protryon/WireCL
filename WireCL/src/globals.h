/*
 * gloabls.h
 *
 *  Created on: Mar 5, 2016
 *      Author: root
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <GLFW/glfw3.h>
#include <CL/cl.h>

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

cl_program wire_program;
cl_command_queue wire_command_queue;
cl_context wire_context;

#define PI 3.141592653589793

#ifdef __MINGW32__
#define INSTALLDIR ""
#else
#define INSTALLDIR ""
#endif

#endif /* GLOBALS_H_ */
