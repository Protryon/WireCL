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
					uint8_t value = 0;
					if (lines[i][x] == '#') value = 1;
					else if (lines[i][x] == '@') value = 2;
					else if (lines[i][x] == '~') value = 3;
					world->data[i * world->width / 4 + x / 4] |= (value << ((x % 4) * 2));
				} else world->data[i * world->width / 4 + x / 4] |= (0 << ((x % 4) * 2));
			}
			free(lines[i]);
		} else {
			for (size_t x = 0; x < world->width; x++) {
				world->data[i * world->width / 4 + x / 4] |= (0 << ((x % 4) * 2));
			}
		}
	}
	free(lines);
	close(fd);
	return 0;
}

int saveWorldText(char* file, struct world* world) {
	int fd = open(file, O_RDWR | O_CREAT, 0644);
	if (fd < 0) return -1;
	char tline[world->width + 1];
	tline[world->width] = 0;
	for (size_t y = 0; y < world->height; y++) {
		for (size_t x = 0; x < world->width; x++) {
			uint8_t v = world->data[y * world->width / 4 + x / 4] & (0x03 << ((x % 4) * 2));
			v >>= (x % 4) * 2;
			char c = ' ';
			if (v == 1) c = '#';
			else if (v == 2) c = '@';
			else if (v == 3) c = '~';
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

