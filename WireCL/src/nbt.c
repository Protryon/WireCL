/*
 * nbt.c
 *
 *  Created on: Mar 25, 2016
 *      Author: root
 */

#include "nbt.h"
#include <stdlib.h>
#include <string.h>

void freeNBT(struct nbt_tag* nbt) {
	if (nbt->name != NULL) free(nbt->name);
	if (nbt->children != NULL) {
		for (size_t i = 0; i < nbt->children_count; i++) {
			freeNBT(nbt->children[i]);
			free(nbt->children[i]);
		}
		free(nbt->children);
	}
	if (nbt->id == NBT_TAG_BYTEARRAY) {
		free(nbt->data.nbt_bytearray.data);
	} else if (nbt->id == NBT_TAG_STRING) {
		free(nbt->data.nbt_string);
	} else if (nbt->id == NBT_TAG_INTARRAY) {
		free(nbt->data.nbt_intarray.ints);
	}
}

struct nbt_tag* cloneNBT(struct nbt_tag* nbt) {
	struct nbt_tag* nt = malloc(sizeof(struct nbt_tag));
	nt->name = nbt->name == NULL ? NULL : strdup(nbt->name);
	nt->id = nbt->id;
	nt->children_count = nbt->children_count;
	nt->children = nt->children_count == 0 ? NULL : malloc(sizeof(struct nbt_tag*) * nt->children_count);
	memcpy(&nt->data, &nbt->data, sizeof(union nbt_data));
	if (nbt->id == NBT_TAG_BYTEARRAY) {
		nt->data.nbt_bytearray.data = malloc(nt->data.nbt_bytearray.len);
		memcpy(nt->data.nbt_bytearray.data, nbt->data.nbt_bytearray.data, nt->data.nbt_bytearray.len);
	} else if (nbt->id == NBT_TAG_STRING) {
		nt->data.nbt_string = strdup(nbt->data.nbt_string);
	} else if (nbt->id == NBT_TAG_INTARRAY) {
		nt->data.nbt_intarray.ints = malloc(nt->data.nbt_intarray.count * 4);
		memcpy(nt->data.nbt_intarray.ints, nbt->data.nbt_intarray.ints, nt->data.nbt_intarray.count * 4);
	}
	for (size_t i = 0; i < nt->children_count; i++) {
		nt->children[i] = cloneNBT(nbt->children[i]);
	}
	return nt;
}
