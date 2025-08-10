#ifndef __ARRAY_T_H__
#define __ARRAY_T_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct array_t
{
    long length;
	long capacity;
    size_t elementSize;
    void* data;
} array_t;


array_t array_create(int initialCapacity, size_t elementSize);
void array_append_element(struct array_t *that, void *element);
void array_insert_element_at(struct array_t *that, void *element, int index);
void *array_get_element_at(array_t that, int index);
void array_delete_element_at(struct array_t *that, int index);
void array_concatenate(struct array_t *that, struct array_t src);
int array_save_to_disk(array_t array, char* file_name);
array_t array_load_from_disk(char* file_name);

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
	array.elementSize = elementSize;
	
	array.data =  ARRAY_MALLOC(size);

    if (!array.data)
    {
        printf("Error allocation memory for Universalarray_t %s::%d\n", __FILE__, __LINE__);
        exit(-1);
    }

	memset(array.data, 0, size);

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
	fwrite(array.data, array.elementSize ,array.length, fp);
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


	fread(&array, sizeof(array_t), 1, fp);

	array.data = ARRAY_MALLOC(array.elementSize * array.capacity);

	fread(array.data, array.elementSize, array.capacity, fp);

	fclose(fp);

	return array;
}

void array_append_element(array_t *that, void *element)
{
	if (that->length + 1 >= that->capacity)
    {
        int size = that->capacity * that->elementSize * 2;
        void* new_pointer = ARRAY_REALLOC(that->data, size);
        if (new_pointer == NULL)
        {
            printf("Error reallocating array\n");
            exit(-1);
        }
        		
		that->capacity *= 2;
		that->data = new_pointer;
		memset((char *)that->data + that->elementSize * that->length, 0, (that->capacity - that->length - 1) * that->elementSize);
    }

    memmove((char *)that->data + that->elementSize * that->length, element, that->elementSize);
    that->length++;
}

void array_insert_element_at(array_t *that, void *element, int index)
{
    if (that->length + 1 >= that->capacity)
    {
        int size = that->capacity * that->elementSize * 2;
        void *new_pointer = ARRAY_REALLOC(that->data, size);
        if (new_pointer == NULL)
        {
            printf("Error reallocating array\n");
            exit(-1);
        }
				
		that->capacity *= 2;
		that->data = new_pointer;
		memset((char *)that->data + that->elementSize * that->length, 0, (that->capacity - that->length - 1) * that->elementSize);

    }

	that->length++;

    memmove((char *)that->data + that->elementSize * (index + 1), 
			(char *)that->data + that->elementSize * index,
			that->elementSize * (that->length - index));

    memmove((char *)that->data + that->elementSize * index, 
			element, that->elementSize);
}

void *array_get_element_at(array_t that, int index)
{
    if (index < that.length)
    {
        return (void *)((char *)that.data  + index * that.elementSize);
    }
    return NULL;
}

void array_delete_element_at(struct array_t* that, int index)
{
	int i;
	for(i = index; i < that->length; i++)
	{
		memmove((char *)that->data + i * that->elementSize, (char *)that->data + (i + 1) * that->elementSize, that->elementSize);
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

#endif
#endif
