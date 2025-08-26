#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <math.h>
#include "../array_t/array_t.h"

typedef struct matrix_t
{
	long size;
	long dimensions;
	array_t data;
} matrix_t;

matrix_t matrix_create(long size, long dimensions, size_t element_size)
{
	matrix_t matrix = {0};

	matrix.size = size;

	matrix.dimensions = dimensions;

	matrix.data = array_create(size * dimensions, element_size);

	return matrix;
}

void* matrix_get_element_at(matrix_t matrix, long *coordinates)
{
	void* element = NULL;
	long lineal_coordinate = 0, i;
	
	for(i = 0; i < matrix.dimensions; i++)
	{
		lineal_coordinate += pow((double)matrix.dimensions, (double)i) + coordinates[i];
	}

	element = array_get_element_at(matrix.data, lineal_coordinate);

	return element;
}

void *matrix_insert_element_at(matrix_t *matrix, void *element, long *coordinates)
{
	long lineal_coordinate = 0, i;
	
	for(i = 0; i < matrix->dimensions; i++)
	{
		lineal_coordinate += pow(matrix->dimensions, i) + coordinates[i];
	}

	return array_insert_element_at(&matrix->data, element, lineal_coordinate);
}

#endif