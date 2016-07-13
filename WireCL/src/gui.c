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
#include "world.h"
#include "render.h"

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
	if (!paused) {
//#ifndef __MINGW32__
		updateWorldGPU (world);
//#else
//		updateWorldCPU (world);
//#endif
	}
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

void drawIngame(float partialTick) {
	float wzoom = width * zoom;
	float hzoom = height * zoom;
	uint8_t v;
	/*uint8_t* rdata = malloc(world->width * world->height * 3);
	 uint8_t r;
	 uint8_t g;
	 uint8_t b;
	 pthread_mutex_lock(&world->swapMutex);
	 for (int32_t y = 0; y < world->height; y++) {
	 for (int32_t x = 0; x < world->width; x++) {
	 v = world_get(world->data, world->width, x, y);
	 //printf("<%i, %i> = %i (%lu)\n", x, y, v, y * world->width * 3 + x * 3);
	 if (v == 1) {
	 r = 0xFF;
	 g = 0xFF;
	 b = 0x00;
	 } else if (v == 2) {
	 r = 0x00;
	 g = 0x00;
	 b = 0xFF;
	 } else if (v == 3) {
	 r = 0xFF;
	 g = 0x00;
	 b = 0x00;
	 } else {
	 r = 0x00;
	 g = 0x00;
	 b = 0x00;
	 }
	 rdata[y * world->width * 3 + x * 3] = r;
	 rdata[y * world->width * 3 + x * 3 + 1] = g;
	 rdata[y * world->width * 3 + x * 3 + 2] = b;
	 }
	 }
	 pthread_mutex_unlock(&world->swapMutex);
	 glEnable (GL_TEXTURE_2D);
	 glBindTexture(GL_TEXTURE_2D, TX_MAP);
	 glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	 #ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
	 glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0);
	 #endif
	 glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, world->width, world->height, 0, GL_RGB, GL_UNSIGNED_BYTE, rdata);
	 free(rdata);*/
	glPushMatrix();
	glTranslatef(-camX + wzoom / 2., -camY + hzoom / 2., 0.);
	pthread_mutex_lock(&world->swapMutex);
	glBegin (GL_QUADS);
	for (int32_t x = (camX - wzoom / 2. - 16) / 16.; x < (camX + wzoom / 2. + 16) / 16.; x++) {
		for (int32_t y = (camY - hzoom / 2. - 16) / 16.; y < (camY + hzoom / 2. + 16) / 16.; y++) {
			v = 0;
			if (x >= 0 && y >= 0 && x < world->width && y < world->height) {
				v = world_get(world->data, world->width, x, y);
			}
			if (v == 0) continue;
			/*if (world->verts[y * world->width * 4 + x * 4].x == 0. && world->verts[y * world->width * 4 + x * 4].y == 0. && world->verts[y * world->width * 4 + x * 4 + 2].x == 0. && world->verts[y * world->width * 4 + x * 4 + 2].y == 0.) {
			 printf("make %i, %i\n", x, y);
			 struct vertex* verts = &world->verts[y * world->width * 4 + x * 4];
			 virtVertex2f(&verts[0], x * 16., y * 16.);
			 virtVertex2f(&verts[1], x * 16., (y + 1.) * 16.);
			 virtVertex2f(&verts[2], (x + 1.) * 16., (y + 1.) * 16.);
			 virtVertex2f(&verts[3], (x + 1.) * 16., y * 16.);
			 glBindVertexArray(world->vao->vao);
			 glBindBuffer(GL_ARRAY_BUFFER, world->vao->vbo);
			 glBufferSubData(GL_ARRAY_BUFFER, y * world->width * 4 * sizeof(struct vertex) + x * 4 * sizeof(struct vertex), sizeof(struct vertex) * 4, verts);
			 }*/
			if (v == 1) glColor3f(1., 1., 0.);
			else if (v == 2) glColor3f(0., 0., 1.);
			else if (v == 3) glColor3f(1., 0., 0.);
			glVertex2f(x * 16., y * 16.);
			glVertex2f(x * 16., (y + 1.) * 16.);
			glVertex2f((x + 1.) * 16., (y + 1.) * 16.);
			glVertex2f((x + 1.) * 16., y * 16.);
		}
	}
	//drawQuads(world->vao);
	glEnd();
	pthread_mutex_unlock(&world->swapMutex);
	//glBegin (GL_QUADS);
	//glVertex2f(0., 0.);
	//glTexCoord2f(0., 1.);
	//glVertex2f(0., world->height * 16.);
	//glTexCoord2f(1., 1.);
	//glVertex2f(world->width * 16., world->height * 16.);
	//glTexCoord2f(1., 0.);
	//glVertex2f(world->width * 16., 0.);
	//glTexCoord2f(0., 0.);
	//glEnd();
	glPopMatrix();
	//glDisable(GL_TEXTURE_2D);
}

void drawGUI(float partialTick) {
	if (guistate == GSTATE_INGAME) {
		drawIngame(partialTick);
	}
}

void gui_keyboardCallback(int key, int scancode, int action, int mods) {
	if (guistate == GSTATE_INGAME) {
		if (key == 32 && action == GLFW_PRESS) {
			paused = !paused;
		}
	}
}

void gui_textCallback(unsigned int codepoint) {
	if (guistate == GSTATE_INGAME) {

	}
}

double lmx = 0.;
double lmy = 0.;

void gui_mouseMotionCallback(double x, double y) {
	if (guistate == GSTATE_INGAME) {
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {
			double dx = lmx - x;
			double dy = lmy - y;
			camX += dx * zoom;
			camY += dy * zoom;
		}
		lmx = x;
		lmy = y;
	}
}

void gui_scrollCallback(double x, double y) {
	if (guistate == GSTATE_INGAME) {
		zoom -= y;
		if (zoom < 1.) zoom = 1.;
	}
}

void claimMouse() {
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void unclaimMouse() {
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

