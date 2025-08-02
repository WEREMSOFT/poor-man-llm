#ifndef __NODE_T_H__
#define __NODE_T_H__

#include "../array_t/array_t.h"

typedef struct node_t
{
	long token;
	array_t children;
} node_t;

node_t node_create()
{
	node_t node = {0};

	node.token = -1;
	node.children = array_create(5, sizeof(long));

	return node;
}

#endif
