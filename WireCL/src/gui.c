/*
 * gui.c
 *
 *  Created on: Mar 5, 2016
 *      Author: root
 */
#include "gui.h"
#include <stdlib.h>
#ifdef __MINGW32__
#define GLEW_STATIC
#include <GL/glew.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include "render.h"
#include <math.h>
#include "globals.h"
#include <fcntl.h>
#include <stdio.h>
#include <png.h>

#define GSTATE_INGAME 0

int guistate = GSTATE_INGAME;

struct __attribute__((__packed__)) rpix {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
};

//int32_t tb_cursor_counter = 0;

void gui_tick() {
	//tb_cursor_counter++;
}

void loadGUI() {
	for (int32_t i = 0; i < 32; i++) {
		int32_t v6 = (i >> 3 & 1) * 85;
		int32_t v7 = (i >> 2 & 1) * 170 + v6;
		int32_t v8 = (i >> 1 & 1) * 170 + v6;
		int32_t v9 = (i >> 0 & 1) * 170 + v6;
		if (i == 6) {
			v7 += 85;
		}
		if (i >= 16) {
			v7 /= 4;
			v8 /= 4;
			v9 /= 4;
		}
		fontColors[i][0] = v7;
		fontColors[i][1] = v8;
		fontColors[i][2] = v9;
	}
	FILE* fd = fopen(INSTALLDIR "assets/ascii.png", "rb");
	if (!fd) goto ppn;
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) {
		fclose (fd);
		goto ppn;
	}
	png_infop info = png_create_info_struct(png);
	if (!info) {
		fclose (fd);
		goto ppn;
	}
	png_init_io(png, fd);
	png_read_info(png, info);
	int width = png_get_image_width(png, info);
	int height = png_get_image_height(png, info);
	png_byte color_type = png_get_color_type(png, info);
	png_byte bit_depth = png_get_bit_depth(png, info);
	if (bit_depth == 16) png_set_strip_16(png);
	if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png);
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png);
	if (png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);
	if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE) png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png);
	png_read_update_info(png, info);
	void* pngd = malloc(height * png_get_rowbytes(png, info));
	png_bytep* row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	for (int y = 0; y < height; y++) {
		row_pointers[y] = (png_byte*) (pngd + (png_get_rowbytes(png, info) * y));
	}
	png_read_image(png, row_pointers);
	free(row_pointers);
	struct rpix* v4 = pngd;
	int cw = width / 16;
	int ch = height / 16;
	for (int i = 0; i < 256; i++) {
		int v10 = i % 16;
		int v11 = i / 16;
		if (i == 32) {
			fontWidth[i] = 4;
			continue;
		}
		int v12 = cw - 1;
		while (1) {
			if (v12 >= 0) {
				int v13 = v10 * cw + v12;
				int v14 = 1;
				for (int v15 = 0; (v15 < ch) && v14; v15++) {
					int v16 = (v11 * cw + v15) * width;
					if (v4[v13 + v16].a > 0) v14 = 0;
				}
				if (v14) {
					--v12;
					continue;
				}
			}
			v12++;
			fontWidth[i] = (unsigned char) (0.5 + ((float) v12 * (8. / cw))) + 1;
			break;
		}
	}
	loadTextureData(TX_ASCII, width, height, pngd, 1);
	free(pngd);
	png_destroy_read_struct(&png, &info, NULL);
	fclose (fd);
	ppn: ;
}
/*
 int drawButton(int x, int y, int width, int height, char* text, int disabled) { // state  is 0 for disabled, 1 for normal, 2 for hovering
 glBindTexture(GL_TEXTURE_2D, TX_WIDGETS);

 int state = disabled ? 0 : ((mouseX > x && mouseX < x + width && mouseY > y && mouseY < y + height) ? 2 : 1);
 glColor4f(1., 1., 1., 1.);
 drawTexturedModalRect(x, y, 0, 0, 46 + state * 20, width / 2, height);
 glColor4f(1., 1., 1., 1.);
 drawTexturedModalRect(x + width / 2, y, 0, 200 - width / 2, 46 + state * 20, width / 2, height);
 uint32_t color = -1;
 if (state == 0) {
 color = 10526880;
 } else if (state == 2) {
 color = 16777120;
 } else {
 color = 14737632;
 }
 glColor4f(1., 1., 1., 1.);
 drawString(text, x + width / 2 - stringWidth(text) / 2, y + height / 2 - 4, color);
 return state == 2;

 }

 int drawTextbox(int x, int y, int width, int height, char* text, int* cursorPos, int highlightStart, int highlightEnd, int scrollOffset, int enabled, int focused) {
 drawRect(x - 1, y - 1, 1, width + 2, height + 2, -6250336);
 drawRect(x, y, 2, width, height, -16777216);
 int v1 = enabled ? 14737632 : 7368816;
 int v2 = *cursorPos - scrollOffset;
 int v3 = highlightEnd - scrollOffset;
 char* ns = strdup(text);
 if (scrollOffset > strlen(ns)) scrollOffset = strlen(ns);
 trimStringToWidth(ns + scrollOffset, width);
 size_t nsl = strlen(ns);
 int v5 = v2 >= 0 && v2 <= nsl;
 int v6 = focused && tb_cursor_counter / 6 % 2 == 0 && v5;
 int v7 = x + 4;
 int v8 = y + (height - 8) / 2;
 if (v3 > nsl) {
 v3 = nsl;
 }
 char tns = 0;
 if (nsl > 0) {
 if (v5) {
 tns = ns[v2];
 ns[v2] = 0;
 }
 drawString(ns, v7, v8, v1);
 }
 int v13 = *cursorPos < nsl || nsl >= 32768;
 int v11 = stringWidth(ns);
 int v9 = v11;
 int os = v11;
 if (!v5) {
 v11 = v2 > 0 ? v7 + width : v7;
 } else if (v13) {
 v11 = v9 - 1;
 --v9;
 }
 if (nsl > 0 && v5 && v2 < nsl) {
 ns[v2] = tns;
 drawString(ns + v2, v7 + os, v8, v1);
 }
 if (v6) {
 if (v13) {
 drawRect(v7 + v11, v8 - 1, 3, 1, 2 + 9, -3092272);
 } else {
 drawString("_", v7 + v11, v8, v1);
 }
 }

 if (mouseX > x && mouseX < x + width && mouseY > y && mouseY < y + height && mouseButton == 0) {
 int rx = mouseX - x - 4;
 trimStringToWidth(ns, rx);
 *cursorPos = strlen(ns) + scrollOffset;
 free(ns);
 return 1;
 } else {
 free(ns);
 return 0;
 }
 }

 void drawMainMenu(float partialTick) {
 //static int fbid = -1;
 glBindTexture(GL_TEXTURE_2D, TX_MMTT);
 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
 //if (fbid == -1) glGenFramebuffers(1, &fbid);
 //glBindFramebuffer(GL_FRAMEBUFFER, fbid);
 //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TX_MMTT, 0);
 //static int rb = -1;
 //if (rb == -1) glGenRenderbuffers(1, &rb);
 //glBindRenderbuffer(GL_RENDERBUFFER, rb);
 //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 256, 256);
 //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb);
 glViewport(0, 0, 256, 256);
 glMatrixMode (GL_PROJECTION);
 glPushMatrix();
 glLoadIdentity();
 gluPerspective(120., 1., .05, 10.);
 glMatrixMode (GL_MODELVIEW);
 glPushMatrix();
 glLoadIdentity();
 glColor4f(1., 1., 1., 1.);
 glRotatef(180., 1., 0., 0.);
 glRotatef(180., 0., 0., 1.);
 glEnable (GL_BLEND);
 glDisable (GL_CULL_FACE);
 glDisable (GL_ALPHA_TEST);
 glDepthMask(0);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 for (int i = 0; i < 64; i++) {
 glPushMatrix();
 float v8 = (fmod(((float) i), 8.) / 7.5) / 64.;
 float v9 = ((((float) i) / 8.) / 7.5) / 64.;
 glTranslatef(v8, v9, 0.);
 glRotatef(-90., 0., 0., 1.);
 glRotatef((sin((mm_pantimer + partialTick) / 400.) * 25.) + 20., 1., 0., 0.);
 glRotatef(-(mm_pantimer + partialTick) * 0.1, 0., 1., 0.);
 for (int x = 0; x < 6; x++) {
 glPushMatrix();
 if (x == 0) {
 glBindTexture(GL_TEXTURE_2D, TX_PAN0);
 } else if (x == 1) {
 glRotatef(90., 0., 1., 0.);
 glBindTexture(GL_TEXTURE_2D, TX_PAN1);
 } else if (x == 2) {
 glRotatef(180., 0., 1., 0.);
 glBindTexture(GL_TEXTURE_2D, TX_PAN2);
 } else if (x == 3) {
 glRotatef(-90., 0., 1., 0.);
 glBindTexture(GL_TEXTURE_2D, TX_PAN3);
 } else if (x == 4) {
 glRotatef(90., 1., 0., 0.);
 glBindTexture(GL_TEXTURE_2D, TX_PAN4);
 } else if (x == 5) {
 glRotatef(-90., 1., 0., 0.);
 glBindTexture(GL_TEXTURE_2D, TX_PAN5);
 }
 glColor4f(1., 1., 1., 1. / (float) (i + 1));
 drawQuads(&mod_pan);
 glPopMatrix();
 }
 glPopMatrix();
 glColorMask(1, 1, 1, 0);
 }
 glColorMask(1, 1, 1, 1);
 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();
 glEnable(GL_CULL_FACE);
 glDepthMask(1);
 for (int i = 0; i < 7; i++) {
 glBindTexture(GL_TEXTURE_2D, TX_MMTT);
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 256, 256);
 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 glColorMask(1, 1, 1, 0);
 glDisable(GL_ALPHA_TEST);
 glBegin (GL_QUADS);
 for (int v5 = 0; v5 < 3; v5++) {
 glColor4f(1., 1., 1., 1. / (float) (v5 + 1));
 float v8 = ((float) (v5 - 3 / 2)) / 256.;
 glTexCoord2f(v8, 1.);
 glVertex3f(swidth, sheight, -1.);
 glTexCoord2f(1. + v8, 1.);
 glVertex3f(swidth, 0., -1.);
 glTexCoord2f(1. + v8, 0.);
 glVertex3f(0., 0., -1.);
 glTexCoord2f(v8, 0.);
 glVertex3f(0., sheight, -1.);
 }
 glEnd();
 glEnable(GL_ALPHA_TEST);
 glColorMask(1, 1, 1, 1);
 }
 glDisable(GL_BLEND);
 glColor4f(1., 1., 1., 1.);
 //glDeleteRenderbuffers(1, &rb);
 glBindFramebuffer(GL_FRAMEBUFFER, 0);
 //glDeleteFramebuffers(1, &fbid);
 glViewport(0, 0, width, height);
 glBindTexture(GL_TEXTURE_2D, TX_MMTT);
 float vr = swidth > sheight ? 120. / (float) swidth : 120. / (float) sheight;
 float ah = (float) sheight * vr / 256.;
 float aw = (float) swidth * vr / 256.;
 glBegin (GL_QUADS);
 glTexCoord2f(.5 - ah, .5 + aw);
 glVertex3f(0., sheight, -.5);
 glTexCoord2f(.5 - ah, .5 - aw);
 glVertex3f(swidth, sheight, -.5);
 glTexCoord2f(.5 + ah, .5 - aw);
 glVertex3f(swidth, 0., -.5);
 glTexCoord2f(.5 + ah, .5 + aw);
 glVertex3f(0., 0., -.5);
 glEnd();
 int ttx = TX_MMTT;
 glDeleteTextures(1, &ttx);
 int v7 = swidth / 2 - 137;
 glBindTexture(GL_TEXTURE_2D, TX_TITLE);
 drawTexturedModalRect(v7, 30, 0, 0, 0, 155, 44);
 drawTexturedModalRect(v7 + 155, 30, 0, 0, 45, 155, 44);
 glPushMatrix();
 glTranslatef((float) swidth / 2. + 90., 70., 1.);
 glRotatef(-20., 0., 0., 1.);
 char* splash = "Minecraft in C!";
 int sw = stringWidth(splash);
 struct timespec ts;
 clock_gettime(CLOCK_MONOTONIC, &ts);
 double ms = (double) ts.tv_nsec / 1000000. + ts.tv_sec * 1000.;
 float v9 = 1.8 - fabs(sin((float) (fmod(ms, 1000.)) / 1000. * PI * 2.) * 0.1);
 v9 = v9 * 100. / (float) (sw + 32);
 glScalef(v9, v9, v9);
 drawString(splash, -sw / 2, -12, -256);
 glPopMatrix();
 drawString("Minecraft 1.9", 2, sheight - 10, -1);
 drawString("Recreated By JavaProphet in C", 2, sheight - 20, -1);
 char* copy = "Copyright Mojang AB. Do not distribute!";
 drawString(copy, swidth - stringWidth(copy) - 2, sheight - 10, -1);
 copy = "Copyright JavaProphet (code)!";
 drawString(copy, swidth - stringWidth(copy) - 2, sheight - 20, -1);
 drawButton(swidth / 2 - 100, sheight / 4 + 48, 200, 20, "Singleplayer", 1);
 if (drawButton(swidth / 2 - 100, sheight / 4 + 48 + 24, 200, 20, "Multiplayer", 0) && mouseButton == 0) {
 guistate = GSTATE_MULTIPLAYER;
 }
 drawButton(swidth / 2 - 100, sheight / 4 + 48 + 48, 200, 20, "Minecraft Realms", 1);
 if (drawButton(swidth / 2 - 100, sheight / 4 + 48 + 72 + 12, 98, 20, "Options", 1) && mouseButton == 0) {
 guistate = GSTATE_OPTIONS;
 }
 if (drawButton(swidth / 2 + 2, sheight / 4 + 48 + 72 + 12, 98, 20, "Quit Game", 0) && mouseButton == 0) {
 exit(0);
 }
 }
 */

void drawIngame(float partialTick) {

}

void drawGUI(float partialTick) {
	if (guistate == GSTATE_INGAME) {
		drawIngame(partialTick);
	}
}

void gui_keyboardCallback(int key, int scancode, int action, int mods) {
	if (guistate == GSTATE_INGAME) {

	}
}

void gui_textCallback(unsigned int codepoint) {
	if (guistate == GSTATE_INGAME) {

	}
}

void gui_mouseMotionCallback(double x, double y) {
	if (guistate == GSTATE_INGAME) {

	}
}

void gui_scrollCallback(double x, double y) {
	if (guistate == GSTATE_INGAME) {

	}
}

void claimMouse() {
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void unclaimMouse() {
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
