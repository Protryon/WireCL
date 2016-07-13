/*
 * world.c
 *
 *  Created on: Jul 12, 2016
 *      Author: root
 */

#ifdef __MINGW32__
#define GLEW_STATIC
#include <GL/glew.h>
#endif
#include <GL/gl.h>
#include <GL/glext.h>
#include "world.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "streams.h"
#ifndef __MINGW32__
#include <CL/cl.h>
#endif
#include "globals.h"
#include "xstring.h"
#include <unistd.h>
#include <pthread.h>
#include "render.h"
#include <time.h>

struct world* newWorld() {
	struct world* world = malloc(sizeof(struct world));
	world->data = NULL;
	world->height = 0;
	world->width = 0;
	world->gps = 0;
	world->generation = 0;
	world->newData = NULL;
	pthread_mutex_init(&world->swapMutex, NULL);
	//world->vao = NULL;
	//world->verts = NULL;
	return world;
}

void freeWorld(struct world* world) {
	pthread_mutex_destroy(&world->swapMutex);
	free(world->data);
	free(world->newData);
	free(world);
}

void world_set(uint8_t *data, int width, int x, int y, uint8_t type) {
	data[y * width / 4 + x / 4] &= ~(0x03 << ((x % 4) * 2)); // clear it first
	data[y * width / 4 + x / 4] |= (type << ((x % 4) * 2));
}

uint8_t world_get(uint8_t *data, int width, int x, int y) {
	uint8_t v = data[y * width / 4 + x / 4] & (0x03 << ((x % 4) * 2));
	v >>= (x % 4) * 2;
	return v;
}

int loadWorldText(char* file, struct world* world) {
	int fd = open(file, O_RDONLY);
	if (fd < 0) return -1;

	char** lines = NULL;
	size_t lineCount = 0;
	char* line = NULL;
	size_t maxWidth = 0;

	do {
		line = NULL;
		if (readLineDynamic(fd, &line) < 0 || line == NULL) break;
		if (lines == NULL) {
			lines = malloc(sizeof(char*));
			lineCount = 0;
		} else {
			lines = realloc(lines, sizeof(char*) * (lineCount + 1));
		}
		lines[lineCount++] = line;
		size_t wid = strlen(line);
		if (wid > maxWidth) maxWidth = wid;
	} while (line != NULL);

	close(fd);

	world->height = lineCount;
	world->width = maxWidth;

	if (world->width % 4 > 0) world->width += (4 - world->width % 4);
	if (world->height % 4 > 0) world->height += (4 - world->height % 4);

	world->data = malloc((world->height * world->width / 4) + 1);
	world->newData = calloc((world->height * world->width / 4) + 1, 1);

	for (size_t i = 0; i < world->height; i++) {
		if (i < lineCount) {
			size_t sl = strlen(lines[i]);
			for (size_t x = 0; x < world->width; x++) {
				if (x < sl) {
					uint8_t value = CELL_NONE;
					if (lines[i][x] == '#') value = CELL_WIRE;
					else if (lines[i][x] == '@') value = CELL_HEAD;
					else if (lines[i][x] == '~') value = CELL_TAIL;
					world_set(world->data, world->width, x, i, value);
				} else world_set(world->data, world->width, x, i, CELL_NONE);
			}
			free(lines[i]);
		} else {
			for (size_t x = 0; x < world->width; x++) {
				world_set(world->data, world->width, x, i, CELL_NONE);
			}
		}
	}
	free(lines);
	//world->vao = calloc(sizeof(struct vao*), 1);
	//world->verts = calloc(sizeof(struct vertex) * world->width * world->height * 4, 1);
	//glGenVertexArrays(1, &world->vao->vao);
	//printf("gvao %i\n", world->vao->vao);
	//glBindVertexArray(world->vao->vao);
	//glGenBuffers(1, world->vao->vbo);
	//glBindBuffer(GL_ARRAY_BUFFER, world->vao->vbo);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(struct vertex) * world->width * world->height * 4, world->verts, GL_STATIC_DRAW);
	//world->vao->vertex_count = world->width * world->height * 4;
	//glVertexPointer(2, GL_FLOAT, sizeof(struct vertex), 0);
	//glEnableClientState (GL_VERTEX_ARRAY);
	return 0;
}

int saveWorldText(char* file, struct world* world) {
	int fd = open(file, O_RDWR | O_CREAT, 0644);
	if (fd < 0) return -1;
	char tline[world->width + 1];
	tline[world->width] = 0;
	for (size_t y = 0; y < world->height; y++) {
		for (size_t x = 0; x < world->width; x++) {
			uint8_t v = world_get(world->data, world->width, x, y);
			char c = ' ';
			if (v == CELL_WIRE) c = '#';
			else if (v == CELL_HEAD) c = '@';
			else if (v == CELL_TAIL) c = '~';
			tline[x] = c;
		}
		writeLine(fd, tline, world->width);
	}
	close(fd);
	return 0;
}

int loadWorldData(char* file, struct world* world) {

}

int saveWorldData(char* file, struct world* world) {

}

#ifndef __MINGW32__
cl_mem inputCL;
cl_mem outputCL;
cl_kernel clk;
double lms = 0.;
void updateWorldGPU(struct world* world) {
	cl_int clret = 0;
	if (!inputCL) {
		inputCL = clCreateBuffer(wire_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR | CL_MEM_ALLOC_HOST_PTR, world->height * world->width / 4, world->data, &clret);
		if (clret != CL_SUCCESS) {
			printf("Error in clCreateBuffer<input>: '%i'\n", clret);
			return;
		}
	}
	if (!outputCL) {
		outputCL = clCreateBuffer(wire_context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, world->height * world->width / 4, NULL, &clret);
		if (clret != CL_SUCCESS) {
			printf("Error in clCreateBuffer<output>: '%i'\n", clret);
			return;
		}
	}
	if (!clk) {
		clk = clCreateKernel(wire_program, "wireworld", &clret);
		if (clret != CL_SUCCESS) {
			printf("Error in clCreateKernel<wireworld>: '%i'\n", clret);
			return;
		}
		clSetKernelArg(clk, 2, sizeof(int), (void *) &world->width);
		clSetKernelArg(clk, 3, sizeof(int), (void *) &world->height);
	}
	clSetKernelArg(clk, 0, sizeof(cl_mem), (void *) &inputCL);
	clSetKernelArg(clk, 1, sizeof(cl_mem), (void *) &outputCL);
	size_t gws[2] = { world->width / 4, world->height };
	clret = clEnqueueNDRangeKernel(wire_command_queue, clk, 2, NULL, gws, NULL, 0, NULL, NULL);
	if (clret != CL_SUCCESS) {
		printf("Error in clEnqueueNDRangeKernel<sobel>: '%i'\n", clret);
		return;
	}
	clFinish (wire_command_queue);
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	double cms = (double) ts.tv_sec * 1000. + (double) ts.tv_nsec / 1000000.;
	int gnd = 0;
	if (cms - lms > (1000. / FRAMELIMIT)) {
		clEnqueueReadBuffer(wire_command_queue, outputCL, CL_TRUE, 0, world->height * world->width / 4, world->newData, 0, NULL, NULL);
		gnd = 1;
		lms = cms;
	}
	//clReleaseMemObject(inputCL);
	//clReleaseMemObject(outputCL);
	//clReleaseKernel (kernel);
	if (gnd) {
		pthread_mutex_lock(&world->swapMutex);
		void* od = world->data;
		world->data = world->newData;
		pthread_mutex_unlock(&world->swapMutex);
		memset(od, 0, world->height * world->width / 4);
		world->newData = od;
	}
	cl_mem tm = outputCL;
	outputCL = inputCL;
	inputCL = tm;
	world->generation++;
	world->gps++;
}
#else
void updateWorldCPU(struct world* world) {
	for (size_t y = 0; y < world->height; y++) {
		for (size_t x = 0; x < world->width; x++) {
			uint8_t oldType = world_get(world->data, world->width, x, y);
			uint8_t newType = CELL_NONE;

			switch (oldType) {
				case CELL_HEAD:
				newType = CELL_TAIL;
				break;
				case CELL_TAIL:
				newType = CELL_WIRE;
				break;
				case CELL_WIRE: {
					uint8_t n = 0;
					int c = 1;

					for (int j = -1; j <= 1 && c; j++) {
						for (int i = -1; i <= 1 && c; i++) {
							size_t k = x + i;
							size_t l = y + j;

							if (k < 0 || l < 0 || k >= world->width || l >= world->height) {
								continue;
							}

							if (world_get(world->data, world->width, k, l) == CELL_HEAD) {
								n++;

								if (n > 2) {
									// micro-optimization
									c = 0;
								}
							}
						}
					}

					newType = (n == 1 || n == 2) ? CELL_HEAD : CELL_WIRE;
					break;
				}
			}

			world_set(world->newData, world->width, x, y, newType);
		}
	}
	pthread_mutex_lock(&world->swapMutex);
	void* od = world->data;
	world->data = world->newData;
	world->newData = od;
	pthread_mutex_unlock(&world->swapMutex);
	world->generation++;
	world->gps++;
}
#endif
