/*
 * render.c
 *
 *  Created on: Feb 23, 2016
 *      Author: root
 */

#include "render.h"
#ifdef __MINGW32__
#define GLEW_STATIC
#include <GL/glew.h>
#endif
#include <GL/gl.h>
#include <GL/glext.h>
#include "xstring.h"
#include <math.h>
#include <time.h>
#include "globals.h"
#include <png.h>

void virtVertex3f(union uvertex* vert, float x, float y, float z) {
	vert->vert.x = x;
	vert->vert.y = y;
	vert->vert.z = z;
}

void virtTexCoord2f(struct vertex_tex* vert, float x, float y) {
	vert->texX = x;
	vert->texY = y;
}

void createVAO(struct vertex* verticies, size_t count, struct vao* vao, int textures, int overwrite, int vattrib) {
	if (!overwrite) glGenVertexArrays(1, &vao->vao);
	glBindVertexArray(vao->vao);
	if (!overwrite) {
		glGenBuffers(1, &vao->vbo);
		glGenBuffers(1, &vao->vib);
	}
	glBindBuffer(GL_ARRAY_BUFFER, vao->vbo);
	glBufferData(GL_ARRAY_BUFFER, count * (textures ? sizeof(struct vertex_tex) : sizeof(struct vertex)), verticies, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao->vib);
	vao->index_count = count; // restart ? (count + (count / restart) - 1) : count;
	vao->vertex_count = count;
	uint16_t indicies[vao->index_count];
	size_t vi = 0;
	for (size_t i = 0; i < vao->index_count; i++) {
		//if (restart && i > 0 && ((i + 1) % (restart + 1)) == 0) {
		//	indicies[i] = 16000;
		//} else {
		indicies[i] = vi++;
		//}
	}
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * vao->index_count, indicies, GL_STATIC_DRAW);
	size_t rz = vattrib ? ((vattrib == GL_BYTE || vattrib == GL_UNSIGNED_BYTE) ? 1 : ((vattrib == GL_SHORT || vattrib == GL_UNSIGNED_SHORT) ? 2 : 4)) : 0;
	glVertexPointer(3, GL_FLOAT, (textures ? sizeof(struct vertex_tex) : sizeof(struct vertex)) + rz, 0);
	if (textures) glTexCoordPointer(2, GL_FLOAT, sizeof(struct vertex_tex) + rz, (void*) (sizeof(struct vertex)));
	glEnableClientState (GL_VERTEX_ARRAY);
	if (textures) glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	if (vattrib) glVertexAttribIPointer(0, 1, vattrib, (textures ? sizeof(struct vertex_tex) : sizeof(struct vertex)) + rz, (void*) (sizeof(struct vertex_tex)));
	glBindVertexArray(0);
	vao->tex = textures;
}

void deleteVAO(struct vao* vao) {
	glDeleteBuffers(1, &vao->vbo);
	glDeleteBuffers(1, &vao->vib);
	glDeleteVertexArrays(1, &vao->vao);
}

void crossp(double* v1, double* v2, double* res) {
	res[0] = v1[1] * v2[2] - v1[2] * v2[1];
	res[1] = v1[2] * v2[0] - v1[0] * v2[2];
	res[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

void drawTriangles(struct vao* vao) {
	if (vao->vertex_count % 3 != 0) return;
	glBindVertexArray(vao->vao);
	glDrawElements(GL_TRIANGLES, vao->index_count, GL_UNSIGNED_SHORT, NULL);
}

void drawQuads(struct vao* vao) {
	if (vao->vertex_count % 4 != 0) return;
	glBindVertexArray(vao->vao);
	glDrawElements(GL_QUADS, vao->index_count, GL_UNSIGNED_SHORT, NULL);
}

void drawChar(char c, int italic) {
	float v3 = c % 16 * 8;
	float v4 = c / 16 * 8;
	float v5 = italic ? 1. : 0.;
	float v6 = fontWidth[c] - .01;
	glBindTexture(GL_TEXTURE_2D, TX_ASCII);
	glBegin (GL_TRIANGLE_STRIP);
	glTexCoord2f(v3 / 128., (v4 + 0.1) / 128.);
	glVertex3f(v5, 0., 0.);
	glTexCoord2f(v3 / 128., (v4 + 7.99 - 0.2) / 128.);
	glVertex3f(-v5, 7.99, 0.);
	glTexCoord2f((v3 + v6 - 1.) / 128., (v4 + 0.1) / 128.);
	glVertex3f(v6 - 1. + v5, 0., 0.);
	glTexCoord2f((v3 + v6 - 1.) / 128., (v4 + 7.99 - 0.2) / 128.);
	glVertex3f(v6 - 1. - v5, 7.99, 0.0);
	glEnd();
}

void drawString(char* str, int x, int y, uint32_t color) {
	size_t sl = strlen(str);
	glDisable (GL_DEPTH_TEST);
	glPushMatrix();
	glTranslatef(x, y, 0.);
	glPushMatrix();
	glTranslatef(1., 1., 0.);
	uint32_t ncolor = (color & 16579836) >> 2 | (color & -16777216);
	glColor4f(((ncolor >> 16) & 0xff) / 255., ((ncolor >> 8) & 0xff) / 255., ((ncolor) & 0xff) / 255., 1.);
	for (size_t i = 0; i < sl; i++) {
		drawChar(str[i], 0);
		glTranslatef(fontWidth[str[i]], 0., 0.);
	}
	glPopMatrix();
	glColor4f(((color >> 16) & 0xff) / 255., ((color >> 8) & 0xff) / 255., ((color) & 0xff) / 255., 1.);
	for (size_t i = 0; i < sl; i++) {
		drawChar(str[i], 0);
		glTranslatef(fontWidth[str[i]], 0., 0.);
	}
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}

size_t stringWidth(char* str) {
	size_t sx = 0;
	size_t sl = strlen(str);
	for (size_t i = 0; i < sl; i++) {
		sx += fontWidth[str[i]];
	}
	return sx;
}

void trimStringToWidth(char* str, size_t width) {
	size_t sx = 0;
	size_t sl = strlen(str);
	for (size_t i = 0; i < sl; i++) {
		sx += fontWidth[str[i]];
		if (sx >= width) {
			str[i] = 0;
			return;
		}
	}
}

void drawTexturedModalRect(int x, int y, int z, int textureX, int textureY, int width, int height) {
	glBegin (GL_QUADS);
	glTexCoord2f(textureX * .00390625, (textureY + height) * .00390625);
	glVertex3f(x, y + height, z);
	glTexCoord2f((textureX + width) * .00390625, (textureY + height) * .00390625);
	glVertex3f(x + width, y + height, z);
	glTexCoord2f((textureX + width) * .00390625, textureY * .00390625);
	glVertex3f(x + width, y, z);
	glTexCoord2f(textureX * .00390625, textureY * .00390625);
	glVertex3f(x, y, z);
	glEnd();
}

void drawRect(int x, int y, int z, int width, int height, uint32_t color) {
	glDisable (GL_TEXTURE_2D);
	glColor4f((float) (color >> 16 & 0xff) / 255., (float) (color >> 8 & 0xff) / 255., (float) (color & 0xff) / 255., (float) (color >> 24 & 0xff) / 255.);
	glBegin (GL_QUADS);
	glVertex3f(x, y + height, z);
	glVertex3f(x + width, y + height, z);
	glVertex3f(x + width, y, z);
	glVertex3f(x, y, z);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glColor4f(1., 1., 1., 1.);
}

int loadTexturePNG(char* path, int id, int s) {
	FILE* fd = fopen(path, "rb");
	if (!fd) return -1;
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) {
		fclose(fd);
		return -1;
	}
	png_infop info = png_create_info_struct(png);
	if (!info) {
		fclose(fd);
		return -1;
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
	loadTextureData(id, width, height, pngd, s);
	free(pngd);
	png_destroy_read_struct(&png, &info, NULL);
	fclose(fd);
	return 0;
}

void loadTextureData(int id, size_t width, size_t height, void* data, int s) {
	glBindTexture(GL_TEXTURE_2D, id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, s ? GL_NEAREST : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0);
#endif
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

