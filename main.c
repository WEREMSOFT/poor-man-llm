#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

#define __ARRAY_T_IMPLEMENTATION__
#include "array_t/array_t.h"

#include "node_t/node_t.h"

void print_words(array_t tokens, array_t token_index);
array_t build_token_graph(array_t tokens, array_t token_index);

int main(void)
{
	struct stat sb;
	char *file_content;
	long i;

	bool first_char = true;
	char null_char = 0;
	
	array_t tokens = array_create(1000, sizeof(char));
	array_t token_index = array_create(1000, sizeof(long));
	array_t token_graph = array_create(100, sizeof(node_t));
	
	int fd = open("libro_test.txt", O_RDONLY);

	if(fd == -1)
	{
		perror("Error al abrir archivo\n");
		return 1;
	}

	if(fstat(fd, &sb) == -1)
	{
		perror("error al obtener el tamanio del archivo\n");
		close(fd);
		return 1;
	}

	file_content = (char *) mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

	if(file_content == MAP_FAILED)
	{
		perror("error al mapear el archivo\n");
		close(fd);
		return 1;
	}
	
	for(i = 0; i < sb.st_size; i++)
	{
		if(file_content[i] != 32 && file_content[i] != '\n')
		{
			if(first_char)
			{
				array_append_element(&token_index, &i);
				first_char = false;
			}
			array_append_element(&tokens, &file_content[i]);
		} else {
			first_char = true;
			array_append_element(&tokens, &null_char);
		}
	}

	print_words(tokens, token_index);

	build_token_graph(tokens, token_index);

	munmap(file_content, sb.st_size);
	close(fd);
		
	return 0;
}

void print_words(array_t tokens, array_t token_index)
{
	long i, *index;
	for(i = 0; i < token_index.length; i++)
	{
		index = (long *)array_get_element_at(token_index, i);
		printf("%s\n", &((char *)tokens.data)[*index]);
	}

	printf("cantidad de palabras %d\n", tokens.length);
}

node_t find_word(char* word, array_t word_graph, array_t tokens)
{
	int i;
	node_t node = {0};
	node_t *element;
	char* current_word;
	
	node.token = -1;

	for(i = 0; i < word_graph.length; i++)
	{
		element = (node_t *)array_get_element_at(word_graph, i);
		current_word =  &((char *)tokens.data)[element->token];
		if(strcmp(current_word, word) == 0)
		{
			return *element;
		}
	}
	
	return node;
}

array_t build_token_graph(array_t tokens, array_t token_index)
{
	long current_word, last_word, *index;
	array_t word_graph = {0};
	node_t word_node;
	
	last_word = -1;

	word_graph = array_create(10, sizeof(node_t));
	
	for(current_word = 0; current_word < token_index.length; current_word++)
	{
		index = (long *)array_get_element_at(token_index, current_word);

		word_node = find_word(&((char *)tokens.data)[*index], word_graph, tokens);

		if(word_node.token == -1)
		{
			word_node = node_create();
			word_node.token = *index;
			array_append_element(&word_graph, &word_node);
			last_word = *index;
			continue;
		}
		
	}

	return word_graph;	
}
