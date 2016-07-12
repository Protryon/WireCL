/*
 * world.c
 *
 *  Created on: Jul 12, 2016
 *      Author: root
 */

#include "world.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "streams.h"

struct world* newWorld() {
	struct world* world = malloc(sizeof(struct world));
	world->data = NULL;
	world->height = 0;
	world->width = 0;
	return world;
}

void freeWorld(struct world* world) {
	free(world->data);
	free(world);
}

void set(uint8_t *data, int width, int x, int y, uint8_t type) {
	data[y * width / 4 + x / 4] |= (type << ((x % 4) * 2));
}

uint8_t get(uint8_t *data, int width, int x, int y) {
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

	for (size_t i = 0; i < world->height; i++) {
		if (i < lineCount) {
			size_t sl = strlen(lines[i]);
			for (size_t x = 0; x < world->width; x++) {
				if (x < sl) {
					uint8_t value = CELL_NONE;
					if (lines[i][x] == '#') value = CELL_WIRE;
					else if (lines[i][x] == '@') value = CELL_HEAD;
					else if (lines[i][x] == '~') value = CELL_TAIL;
					set(world->data, world->width, x, i, value);
				} else set(world->data, world->width, x, i, CELL_NONE);
			}
			free(lines[i]);
		} else {
			for (size_t x = 0; x < world->width; x++) {
				set(world->data, world->width, x, i, CELL_NONE);
			}
		}
	}
	
	free(lines);
	return 0;
}

int saveWorldText(char* file, struct world* world) {
	int fd = open(file, O_RDWR | O_CREAT, 0644);
	if (fd < 0) return -1;
	char tline[world->width + 1];
	tline[world->width] = 0;
	for (size_t y = 0; y < world->height; y++) {
		for (size_t x = 0; x < world->width; x++) {
			uint8_t v = get(world->data, world->width, x, y);
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

void updateWorld(struct world* world) {
	size_t size = world->height * world->width / 4;
	uint8_t *newData = calloc(size + 1, 1);

	for(size_t y = 0; y < world->height; y++) {
		for(size_t x = 0; x < world->width; x++) {
			uint8_t oldType = get(world->data, world->width, x, y);
			uint8_t newType = CELL_NONE;
			
			switch(oldType) {
				case CELL_HEAD:
					newType = CELL_TAIL;
					break;
				case CELL_TAIL:
					newType = CELL_WIRE;
					break;
				case CELL_WIRE: {
					uint8_t n = 0;
					int c = 1;

					for(int j = -1; j <= 1 && c; j++) {
						for(int i = -1; i <= 1 && c; i++) {
							size_t k = x + i;
							size_t l = y + j;

							if(k < 0 || l < 0 || k >= world->width || l >= world->height) {
								continue;
							}	

							if(get(world->data, world->width, k, l) == CELL_HEAD) {
								n++;

								if(n > 2) {
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

			set(newData, world->width, x, y, newType);
		}
	}

	for(int i = 0; i < size; i++) {
		*(world->data + i) = *(newData + i);
	}

	free(newData);
}