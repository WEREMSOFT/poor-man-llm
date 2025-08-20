#ifndef __NODE_T_H__
#define __NODE_T_H__

#include "../array_t/array_t.h"

typedef struct node_t
{
	long token;
	array_t children;
} node_t;

#define NODE_NUM_PARAM 3

typedef struct node_n_t
{
	long token[NODE_NUM_PARAM];
	array_t children;
} node_n_t;

node_n_t node_n_create()
{
	node_n_t node = {0};

	node.token[0] = -1;
	node.children = array_create(5, sizeof(long));
	return node;
}

node_t node_create()
{
	node_t node = {0};

	node.token = -1;
	node.children = array_create(5, sizeof(long));

	return node;
}

#endif
