#ifndef __LZXW_COMPRESS_H__
#define __LZXW_COMPRESS_H__

#include <memory.h>
#include <ctype.h>
#define __ARRAY_T_IMPLEMENTATION__
#include "../array_t/array_t.h"

int lzw_get_dictionary_index(array_t dictionary, array_t sequence)
{
	int i;
	array_t *string;
	for(i = 0; i < dictionary.length; i++)
	{
		string = array_get_element_at(dictionary, i);
		if(strcmp(string->data, sequence.data) == 0)
		{
			return i;
		}
	}
	return -1;
}

array_t compress(char* input_string, array_t *dictionary)
{
	int i;
	int dictionary_index, last_found_index;
	array_t array_to_insert;
	*dictionary = array_create(256, sizeof(array_t));
	array_t string_pattern = {0};
	array_t current_sequence = array_create(10, sizeof(char));
	int in_strlen = strlen(input_string);
	array_t output_string_arr = array_create(in_strlen, sizeof(int));
	char current_char = 0;

	for(i = 0; i < 256; i++)
	{
		string_pattern = array_create(2, sizeof(char));
		array_append_element(&string_pattern, &i);
		array_append_element(dictionary, &string_pattern);
	}

	for(i = 0; i < in_strlen; i++)
	{
		current_char = tolower(input_string[i]);
		array_append_element(&current_sequence, &current_char);
		
		dictionary_index = lzw_get_dictionary_index(*dictionary, current_sequence);

		if(dictionary_index != -1)
		{
			last_found_index = dictionary_index;
			continue;
		} else {
			array_to_insert = array_create(current_sequence.length + 1, sizeof(char));
			strcpy(array_to_insert.data, current_sequence.data);
			array_to_insert.length = current_sequence.length;
			array_append_element(dictionary, &array_to_insert);
			dictionary_index = dictionary->length - 1;
			array_append_element(&output_string_arr, &last_found_index);
			array_reset(&current_sequence);
			i--;
		}
	}
	array_append_element(&output_string_arr, &last_found_index);
	
	return output_string_arr;
}

#endif
