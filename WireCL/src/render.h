/*
 * render.h
 *
 *  Created on: Feb 23, 2016
 *      Author: root
 */

#ifndef RENDER_H_
#define RENDER_H_

#include <stdlib.h>
#include <stdint.h>

unsigned char fontColors[32][3];
unsigned char fontWidth[256];

struct __attribute__((packed)) vertex {
		float x;
		float y;
		float z;
};

struct __attribute__((packed)) vertex_tex {
		float x;
		float y;
		float z;
		float texX;
		float texY;
};

union uvertex {
		struct vertex vert;
		struct vertex_tex tex;
};

struct vao {
		int vao;
		int vbo;
		int tex;
		size_t vertex_count;
		int vib;
		size_t index_count;
};

#define TX_NONE 0
#define TX_ASCII 1

void virtVertex3f(union uvertex* vert, float x, float y, float z);

void virtTexCoord2f(struct vertex_tex* vert, float x, float y);

void createVAO(struct vertex* verticies, size_t count, struct vao* vao, int textures, int overwrite, int vattrib);

void deleteVAO(struct vao* vao);

void drawTriangles(struct vao* vao);

void drawQuads(struct vao* vao);

void drawChar(char c, int italic);

void drawString(char* str, int x, int y, uint32_t color);

size_t stringWidth(char* str);

void trimStringToWidth(char* str, size_t width);

void drawTexturedModalRect(int x, int y, int z, int textureX, int textureY, int width, int height);

void drawRect(int x, int y, int z, int width, int height, uint32_t color);

int loadTexturePNG(char* path, int id, int s);

void loadTextureData(int id, size_t width, size_t height, void* data, int s);

#endif /* RENDER_H_ */
