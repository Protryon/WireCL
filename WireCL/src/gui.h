/*
 * gui.h
 *
 *  Created on: Mar 5, 2016
 *      Author: root
 */

#ifndef GUI_H_
#define GUI_H_

void drawGUI(float partialTick);

void gui_tick();

void loadGUI();

void claimMouse();

void unclaimMouse();

void gui_keyboardCallback(int key, int scancode, int action, int mods);

void gui_textCallback(unsigned int codepoint);

void gui_mouseMotionCallback(double x, double y);

void gui_mouseCallback(int button, int action, int mods);

void gui_scrollCallback(double x, double y);

//int drawButton(int x, int y, int width, int height, char* text, int disabled);

#endif /* GUI_H_ */
