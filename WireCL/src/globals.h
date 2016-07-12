/*
 * gloabls.h
 *
 *  Created on: Mar 5, 2016
 *      Author: root
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <GLFW/glfw3.h>

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

#define PI 3.141592653589793

#ifdef __MINGW32__
#define INSTALLDIR ""
#else
#define INSTALLDIR ""
#endif

#endif /* GLOBALS_H_ */
