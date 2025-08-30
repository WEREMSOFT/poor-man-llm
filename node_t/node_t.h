#ifndef __NODE_T_H__
#define __NODE_T_H__

#include "../array_t/array_t.h"

#define NODE_NUM_PARAM 3

typedef struct node_t
{
	int key[NODE_NUM_PARAM];
	array_t children;
} node_t;

node_t node_create()
{
	node_t node = {0};

	node.key[0] = -1;
	node.children = array_create(5, sizeof(long[NODE_NUM_PARAM]));
	return node;
}

#endif
