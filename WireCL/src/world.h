/*
 * world.h
 *
 *  Created on: Jul 12, 2016
 *      Author: root
 */

#ifndef WORLD_H_
#define WORLD_H_

#include <stdint.h>

struct world {
		uint8_t* data;
		uint32_t width;
		uint32_t height;
		//total size is width * height / 4, pos index is [y * width + x / 4] & (0x03 << ((x % 4) * 2))
		//set by [y * width + x / 4] = ~([y * width + x / 4] & (0x03 << ((x % 4) * 2))) | value
		// 0 = empty 1 = wire 2 = electron head 3 = electron tail
};

struct world* newWorld();

void freeWorld(struct world* world);

int loadWorldText(char* file, struct world* world);

int saveWorldText(char* file, struct world* world);

int loadWorldData(char* file, struct world* world);

int saveWorldData(char* file, struct world* world);

#endif /* WORLD_H_ */
