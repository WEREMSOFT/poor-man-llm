#ifndef __ARRAY_T_H__
#define __ARRAY_T_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct array_t
{
    int length;
	int capacity;
    size_t element_size;
    void* data;
} array_t;


array_t array_create(int initialCapacity, size_t elementSize);
void* array_append_element(struct array_t *that, void *element);
void* array_insert_element_at(array_t *that, void *element, int index);
void *array_insert_element_in_order(struct array_t *that, void *element, int (*compar)(const void *, const void *, void *));
void *array_get_element_at(array_t that, int index);
void array_delete_element_at(struct array_t *that, int index);
void array_concatenate(struct array_t *that, struct array_t src);
int array_save_to_disk(array_t array, char* file_name);
array_t array_load_from_disk(char* file_name);
array_t array_destroy(array_t array);
void *array_search_element(struct array_t array, void *key, int (*compar)(const void *, const void *));

#ifndef ARRAY_MALLOC
#define ARRAY_MALLOC malloc
#endif

#ifndef ARRAY_REALLOC
#define ARRAY_REALLOC realloc
#endif

#ifdef __ARRAY_T_IMPLEMENTATION__
#undef __ARRAY_T_IMPLEMENTATION__

array_t array_create(int initialCapacity, size_t elementSize)
{
    size_t size = elementSize * initialCapacity;
    array_t array = {0};
    
	array.length = 0;
	array.capacity = initialCapacity;
	array.element_size = elementSize;
	
	array.data =  ARRAY_MALLOC(size);

    if (!array.data)
    {
        printf("Error allocation memory for Universalarray_t %s::%d\n", __FILE__, __LINE__);
        exit(-1);
    }

	memset(array.data, 0, size);

    return array;
}

array_t array_destroy(array_t array)
{
	free(array.data);
	array.capacity = array.element_size = array.length  = 0;
	array.data = NULL;
	return array;
}

int array_save_to_disk(array_t array, char* file_name)
{
	FILE *fp = fopen(file_name, "w+");

	if(!fp)
	{
		printf("Error trying to save array to file %s\n", file_name);
		return -1;
	}

	fwrite(&array, sizeof(array_t), 1, fp);
	fwrite(array.data, array.element_size ,array.length, fp);
	fclose(fp);

	return 0;
}

array_t array_load_from_disk(char* file_name)
{
	array_t array = {0};
	
	FILE *fp = fopen(file_name, "r");

	if(!fp)
	{
		printf("Error trying to load array to file %s\n", file_name);
		return array;
	}

	(void)fread(&array, sizeof(array_t), 1, fp);
	
	array.data = ARRAY_MALLOC(array.element_size * array.capacity);

	(void) fread(array.data, array.element_size, array.capacity, fp);

	fclose(fp);

	return array;
}

void* array_append_element(array_t *that, void *element)
{
	if (that->length + 1 >= that->capacity)
    {
        int size = that->capacity * that->element_size * 2;
        void* new_pointer = ARRAY_REALLOC(that->data, size);
        if (new_pointer == NULL)
        {
            printf("Error reallocating array\n");
            exit(-1);
        }
        		
		that->capacity *= 2;
		that->data = new_pointer;
		memset((char *)that->data + that->element_size * that->length, 0, (that->capacity - that->length - 1) * that->element_size);
    }

    memmove((char *)that->data + that->element_size * that->length, element, that->element_size);
    that->length++;
	return (char *)((char *)that->data + that->element_size * (that->length - 1));
}

void* array_insert_element_at(array_t *that, void *element, int index)
{
	if(index > that->length)
	{
		printf("index out of bounds\n");
		exit(-1);
	}
    if (that->length + 1 >= that->capacity)
    {
        int size = that->capacity * that->element_size * 2;
        void *new_pointer = ARRAY_REALLOC(that->data, size);
        if (new_pointer == NULL)
        {
            printf("Error reallocating array\n");
            exit(-1);
        }
				
		that->capacity *= 2;
		that->data = new_pointer;
		memset((char *)that->data + that->element_size * that->length, 0, (that->capacity - that->length - 1) * that->element_size);
    }

	that->length++;

    memmove((char *)that->data + that->element_size * (index + 1), 
			(char *)that->data + that->element_size * index,
			that->element_size * (that->length - index));

    memmove((char *)that->data + that->element_size * index, 
			element, that->element_size);

	return (char *)that->data + that->element_size * index;
}

void *array_get_element_at(array_t that, int index)
{
    if (index < that.length)
    {
        return (void *)((char *)that.data  + index * that.element_size);
    }
    return NULL;
}

void array_delete_element_at(struct array_t* that, int index)
{
	int i;
	for(i = index; i < that->length; i++)
	{
		memmove((char *)that->data + i * that->element_size, (char *)that->data + (i + 1) * that->element_size, that->element_size);
	}
	that->length--;
}

void array_concatenate(struct array_t *that, struct array_t src)
{
	int i;
	for(i = 0; i < src.length; i++)
	{
		array_append_element(that, (char *)src.data + i);
	}
}

void* array_insert_element_in_order(struct array_t *that, void *element, int (*compar)(const void *, const void *, void *))
{
	int i, result;
	void *temp_element;
	/*
	element = array_append_element(that, element);
	*/ 
	for(i = 0; i < that->length; i++)
	{
		temp_element = array_get_element_at(*that, i);
		result = compar(temp_element, element, NULL);
		if(result >= 0)
		{
			return array_insert_element_at(that, element, i);
		}
	}
	/*
	qsort_r(that->data, that->length, that->element_size, compar, NULL);
	*/
	element = array_append_element(that, element);
	return element;
}

void *array_search_element(struct array_t array, void *key, int (*compar)(const void *, const void *))
{
	return bsearch(key, array.data, array.length, array.element_size, compar);
}
#endif
#endif
