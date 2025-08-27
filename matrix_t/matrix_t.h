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

long get_lineal_coordinate(long dimensions, long size, long* coordinates)
{
	long lineal_coordinate = 0, i, j;
	
	j = dimensions;
	for(i = 0; i < dimensions; i++)
	{
		j--;
		lineal_coordinate += pow((double)size, (double)j) * coordinates[i];
	}

	return lineal_coordinate;
}

void* matrix_get_element_at(matrix_t matrix, long *coordinates)
{
	void* element = NULL;
	long lineal_coordinate = 0;
	
	lineal_coordinate = get_lineal_coordinate(matrix.dimensions, matrix.size, coordinates);

	element = array_get_element_at(matrix.data, lineal_coordinate);

	return element;
}

void *matrix_insert_element_at(matrix_t *matrix, void *element, long *coordinates)
{
	long lineal_coordinate = 0;
	array_t* long_array = {0};
	
	lineal_coordinate = get_lineal_coordinate(matrix->dimensions, matrix->size, coordinates);

	long_array = array_get_element_at(matrix->data, lineal_coordinate);

	if(long_array->data == NULL)
	{
		*long_array = array_create(10, sizeof(long));
	}

	return array_append_element(long_array, element);
}

int matrix_save_to_disk(matrix_t matrix, char* matrix_file_name, char* array_file_name)
{
	long i;
	array_t *temp_array;
	FILE *fp = fopen(matrix_file_name, "w+");
	char file_name[500] = {0};

	if(!fp)
	{
		printf("Error trying to save array to file %s\n", matrix_file_name);
		return -1;
	}

	fwrite(&matrix, sizeof(matrix_t), 1, fp);
	fclose(fp);

	array_save_to_disk(matrix.data, array_file_name);

	for(i = 0; i < matrix.data.length; i++)
	{
		sprintf(file_name, "arr-%ld.arr", i);
		temp_array = array_get_element_at(matrix.data, i);
		array_save_to_disk(*temp_array, file_name);
	}

	return 0;
}

matrix_t matrix_load_from_disk(char* matrix_file_name, char* array_file_name)
{
	matrix_t matrix = {0};
	
	FILE *fp = fopen(matrix_file_name, "r");

	if(!fp)
	{
		printf("Error trying to load array to file %s\n", matrix_file_name);
		return matrix;
	}


	fread(&matrix, sizeof(matrix_t), 1, fp);
	fclose(fp);

	matrix.data = array_load_from_disk(array_file_name);

	return matrix;
}

#endif