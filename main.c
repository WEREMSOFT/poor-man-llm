#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define __ARRAY_T_IMPLEMENTATION__
#include "array_t/array_t.h"

#include "node_t/node_t.h"

void print_words(array_t tokens, array_t token_index);
array_t build_token_graph(array_t tokens, array_t token_index);
void generate_phrase(array_t word_graph, char* initial_word, array_t tokens);
node_t* find_word(char* word, array_t word_graph, array_t tokens);

int main(void)
{
	struct stat sb;
	char *file_content;
	long i, temp_index;

	bool first_char = true;
	char null_char = 0;
	array_t token_graph;

	
	
	array_t tokens = array_create(1000, sizeof(char));
	array_t token_index = array_create(1000, sizeof(long));
	
	int fd = open("libro.txt", O_RDONLY);

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
		if(file_content[i] != 32 && file_content[i] != '\n' && file_content[i] != '\r' && file_content[i] != '.')
		{
			if(first_char)
			{
				temp_index = tokens.length;
				array_append_element(&token_index, &temp_index);
				first_char = false;
			}
				
			array_append_element(&tokens, &file_content[i]);
		} else {
			first_char = true;

			if(file_content[i] == '.')
			{
				array_append_element(&tokens, &null_char);
				array_append_element(&tokens, &file_content[i]);
				temp_index = tokens.length - 1;
				array_append_element(&token_index, &temp_index);
			}

			array_append_element(&tokens, &null_char);
		}
	}

	token_graph = build_token_graph(tokens, token_index);

	generate_phrase(token_graph, "The", tokens);
	printf("\n");
	generate_phrase(token_graph, "If", tokens);
	printf("\n");
	generate_phrase(token_graph, "Then", tokens);
	printf("\n");
	generate_phrase(token_graph, "Although", tokens);
	printf("\n");
	generate_phrase(token_graph, "Finally", tokens);

	munmap(file_content, sb.st_size);
	close(fd);
		
	return 0;
}

void generate_phrase(array_t word_graph, char* initial_word, array_t tokens)
{
	long random_index, *word_index;
	node_t* last_word_node = find_word(initial_word, word_graph, tokens);

	srand((unsigned int)time(NULL));

	while(last_word_node != NULL)
	{
		if(((char *)tokens.data)[last_word_node->token] != '.')
		{
			printf(" ");
		}

		printf("%s", &((char *)tokens.data)[last_word_node->token]);

		if(last_word_node->children.length == 0) return;

		if(((char *)tokens.data)[last_word_node->token] == '.')
		{
			printf("\n");
			return;
		}

		random_index = rand() % last_word_node->children.length;
		word_index = array_get_element_at(last_word_node->children, random_index);
		last_word_node = find_word(&((char *)tokens.data)[*word_index], word_graph, tokens);
	}
	
}

void print_words(array_t tokens, array_t token_index)
{
	long i, *index;
	for(i = 0; i < token_index.length; i++)
	{
		index = (long *)array_get_element_at(token_index, i);
		printf("%s\n", &((char *)tokens.data)[*index]);
	}

	printf("cantidad de palabras %d\n", token_index.length);
}

node_t* find_word(char* word, array_t word_graph, array_t tokens)
{
	int i;
	node_t *element = NULL;
	char* current_word;

	for(i = 0; i < word_graph.length; i++)
	{
		element = (node_t *)array_get_element_at(word_graph, i);
		current_word =  &((char *)tokens.data)[element->token];
		if(strcmp(current_word, word) == 0)
		{
			return element;
		}
	}
	
	return NULL;
}

node_t* word_append_to_graph(array_t* word_graph, long *index)
{
	node_t word_node = {0};
	word_node = node_create();
	word_node.token = *index;
	array_append_element(word_graph, &word_node);
	return array_get_element_at(*word_graph, word_graph->length - 1);
}

array_t build_token_graph(array_t tokens, array_t token_index)
{
	long current_word, *index, last_word_index;
	array_t word_graph = {0};
	node_t *last_word_node;

	word_graph = array_create(10, sizeof(node_t));
	
	for(current_word = 0; current_word < token_index.length; current_word++)
	{
		index = (long *)array_get_element_at(token_index, current_word);

		/* add  */
		if(current_word == 0)
		{
			word_append_to_graph(&word_graph, index);
			last_word_index = *index;
			continue;
		}
		/* find / add - add child  */
		last_word_node = find_word(&((char *)tokens.data)[last_word_index], word_graph, tokens);

		if(last_word_node == NULL)
		{
			last_word_node = word_append_to_graph(&word_graph, &last_word_index);
		}

		array_append_element(&last_word_node->children, index);
		last_word_index = *index;
	}

	return word_graph;	
}
