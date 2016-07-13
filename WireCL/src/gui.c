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
	if (!paused || skr > 0) {
//#ifndef __MINGW32__
		updateWorldGPU (world);
		if (skr > 0) skr--;
//#else
//		updateWorldCPU (world);
//#endif
	}
}

int delrad = 1;

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

int sfs = 0;
double lmx = 0.;
double lmy = 0.;

void screenToWorld(double* x, double* y) {
	*x = ((camX - (float) width * zoom / 2. + (float) *x * zoom) / 16.);
	*y = ((camY - (float) height * zoom / 2. + (float) *y * zoom) / 16.);
}

void screenToWorldInt(int32_t* x, int32_t* y) {
	*x = (int32_t)((camX - (float) width * zoom / 2. + (float) *x * zoom) / 16.);
	*y = (int32_t)((camY - (float) height * zoom / 2. + (float) *y * zoom) / 16.);
}

void drawIngame(float partialTick) {
	float wzoom = width * zoom;
	float hzoom = height * zoom;
	uint8_t v;
	static uint8_t* rdata = NULL;
	if (!rdata) rdata = malloc(world->width * world->height * 3);
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0);
#endif
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, world->width, world->height, 0, GL_RGB, GL_UNSIGNED_BYTE, rdata);
	glPushMatrix();
	glTranslatef(-camX + wzoom / 2., -camY + hzoom / 2., 0.);
	/*pthread_mutex_lock(&world->swapMutex);
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
	 }* /
	 if (v == 1) glColor3f(1., 1., 0.);
	 else if (v == 2) glColor3f(0., 0., 1.);
	 else if (v == 3) glColor3f(1., 0., 0.);
	 glVertex2f(x * 16., y * 16.);
	 glVertex2f(x * 16., (y + 1.) * 16.);
	 glVertex2f((x + 1.) * 16., (y + 1.) * 16.);
	 glVertex2f((x + 1.) * 16., y * 16.);
	 }
	 }
	 pthread_mutex_unlock(&world->swapMutex);*/
	glBegin (GL_QUADS);
	glVertex2f(0., 0.);
	glTexCoord2f(0., 1.);
	glVertex2f(0., world->height * 16.);
	glTexCoord2f(1., 1.);
	glVertex2f(world->width * 16., world->height * 16.);
	glTexCoord2f(1., 0.);
	glVertex2f(world->width * 16., 0.);
	glTexCoord2f(0., 0.);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	if (sfs && paused) {
		int32_t mx = (int32_t) lmx;
		int32_t my = (int32_t) lmy;
		screenToWorldInt(&mx, &my);
		double rx = (double) mx + .5;
		double ry = (double) my + .5;
		glColor3f(0., 1., 0.);
		glBegin(GL_QUADS);
		glVertex2f((rx - delrad + .5) * 16., (ry - delrad + .5) * 16.);
		glVertex2f((rx - delrad + .5) * 16., (ry + delrad - .5) * 16.);
		glVertex2f((rx + delrad - .5) * 16., (ry + delrad - .5) * 16.);
		glVertex2f((rx + delrad - .5) * 16., (ry - delrad + .5) * 16.);
		glEnd();
		glColor3f(1., 1., 1.);
	}
	//drawQuads(world->vao);
	//glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

void drawGUI(float partialTick) {
	if (guistate == GSTATE_INGAME) {
		drawIngame(partialTick);
	}
}

void gui_keyboardCallback(int key, int scancode, int action, int mods) {
	if (guistate == GSTATE_INGAME) {
		if (key == 32 && action == GLFW_PRESS) {
			if (paused == 2) {
				reflush = 1;
			}
			paused = !paused;
		} else if (key == 83) {
			if (paused && action == GLFW_PRESS) {
				if (paused == 2) {
					reflush = 1;
					paused = 1;
				}
				skr++;
			}
		} else if (paused) {
			if (key == 341) {
				if (action == GLFW_PRESS) sfs = 1;
			}
		}
		if (key == 341 && action == GLFW_RELEASE) sfs = 0;

	}
}

void gui_textCallback(unsigned int codepoint) {
	if (guistate == GSTATE_INGAME) {

	}
}

void placeAtPos(int32_t x, int32_t y, uint8_t value) {
	screenToWorldInt(&x, &y);
	if (x < 0 || y < 0 || x >= world->width || y >= world->height) {
		return;
	}
	world_set(world->data, world->width, x, y, value);
}

double lgpx = -1.;
double lgpy = -1.;
int bp = 0;
void gui_mouseMotionCallback(double x, double y) {
	if (guistate == GSTATE_INGAME) {
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {
			double dx = lmx - x;
			double dy = lmy - y;
			camX += dx * zoom;
			camY += dy * zoom;
		}
		if (paused && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) && !bp) {
			double lx = x;
			double ly = y;
			screenToWorld(&lx, &ly);
			double ang = atan2(ly - lgpy, lx - lgpx);
			double lx2 = lgpx;
			double ly2 = lgpy;
			if (lx2 == -1.) lx2 = lx;
			if (ly2 == -1.) ly2 = ly;
			//printf("angle %f\n", ang);
			double acos = cos(ang); // not a arccos/arcsin!!!!!!!
			double asin = sin(ang);
			int c = 0;
			while (c++ < 50) {
				//printf("%i: %f, %f\n", c, lp2xx, lp2yy);
				int32_t px = (int32_t) lx2;
				int32_t py = (int32_t) ly2;
				if (px < 0 || py < 0 || px >= world->width || py >= world->height) continue;
				if (lx2 < 0 || ly2 < 0 || lx2 >= world->width || ly2 >= world->height) {
					return;
				}
				if (sfs) {
					for (int32_t zx = px - delrad + 1; zx <= px + delrad - 1; zx++) {
						for (int32_t zy = py - delrad + 1; zy <= py + delrad - 1; zy++) {
							if (zx < 0 || zy < 0 || zx >= world->width || zy >= world->height) continue;
							world_set(world->data, world->width, zx, zy, 0);
						}
					}
				} else {
					world_set(world->data, world->width, lx2, ly2, CELL_WIRE);
				}
				if (fabs(lx2 - lx) < 1. && fabs(ly2 - ly) < 1.) {
					c = 50;
				}
				lx2 += acos;
				ly2 += asin;
			}
			//printf("%i extra set\n", c);
			paused = 2;
			lgpx = lx;
			lgpy = ly;
		} else {
			lgpx = -1.;
			lgpy = -1.;
		}
		lmx = x;
		lmy = y;
	}
}

void gui_mouseCallback(int button, int action, int mods) {
	if (paused) {
		if (button == 1 && action == GLFW_PRESS) {
			if (mods & GLFW_MOD_CONTROL) {
				int32_t mx = (int32_t) lmx;
				int32_t my = (int32_t) lmy;
				screenToWorldInt(&mx, &my);
				for (int32_t x = mx - delrad + 1; x <= mx + delrad - 1; x++) {
					for (int32_t y = my - delrad + 1; y <= my + delrad - 1; y++) {
						if (x < 0 || y < 0 || x >= world->width || y >= world->height) continue;
						world_set(world->data, world->width, x, y, 0);
					}
				}
				paused = 2;
			} else {
				placeAtPos(lmx, lmy, (mods & GLFW_MOD_SHIFT) ? CELL_HEAD : CELL_WIRE);
				paused = 2;
				bp = mods & GLFW_MOD_SHIFT;
			}
		}
	}
}

void gui_scrollCallback(double x, double y) {
	if (guistate == GSTATE_INGAME) {
		if (paused && sfs) {
			delrad += y;
			if (delrad < 1.) delrad = 1.;
		} else {
			zoom -= y;
			if (zoom < 1.) zoom = 1.;
		}
	}
}

void claimMouse() {
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void unclaimMouse() {
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

