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

	matrix.data = array_create(pow(size, dimensions), element_size);

	matrix.data.length = matrix.data.capacity;

	return matrix;
}

long get_lineal_coordinate(long dimensions, long* coordinates)
{
	long lineal_coordinate = 0, i, j;
	
	j = dimensions;
	for(i = 0; i < dimensions; i++)
	{
		j--;
		lineal_coordinate += pow((double)dimensions, (double)j) * coordinates[i];
	}

	return lineal_coordinate;
}

void* matrix_get_element_at(matrix_t matrix, long *coordinates)
{
	void* element = NULL;
	long lineal_coordinate = 0;
	
	lineal_coordinate = get_lineal_coordinate(matrix.dimensions, coordinates);

	element = array_get_element_at(matrix.data, lineal_coordinate);

	return element;
}

void *matrix_insert_element_at(matrix_t *matrix, void *element, long *coordinates)
{
	long lineal_coordinate = 0;
	array_t* long_array = {0};
	
	lineal_coordinate = get_lineal_coordinate(matrix->dimensions, coordinates);

	long_array = array_get_element_at(matrix->data, lineal_coordinate);

	if(long_array->data == NULL)
	{
		*long_array = array_create(10, sizeof(long));
	}

	return array_append_element(long_array, element);
}

#endif