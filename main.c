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
void generate_tokens(array_t* tokens, array_t* token_index);
void print_graph(array_t graph, array_t tokens);

int main(void)
{
	array_t token_graph = {0};
	array_t tokens = {0};	
	array_t token_index = {0};
	node_t *node_temp;
	char file_name[500] = {0};
	long i;
	
	tokens = array_load_from_disk("model_data/tokens.arr");
	token_index = array_load_from_disk("model_data/token_index.arr");

	if(tokens.length == 0 || token_index.length == 0)
	{
		generate_tokens(&tokens, &token_index);
	}

	token_graph = array_load_from_disk("model_data/token_graph.arr");

	if(token_graph.length != 0)
	{
		for( i = 0; i < token_graph.length; i++)
		{
			node_temp = array_get_element_at(token_graph, i);
			sprintf(file_name, "model_data/token_graph_%ld.arr", i);
			node_temp->children = array_load_from_disk(file_name);
		}
	}

	if(token_graph.length == 0)
	{
		token_graph = build_token_graph(tokens, token_index);
	}

	print_graph(token_graph, tokens);

	printf("\n");
	generate_phrase(token_graph, "If", tokens);
	printf("\n");
	generate_phrase(token_graph, "Then", tokens);
	printf("\n");
	generate_phrase(token_graph, "Although", tokens);
	printf("\n");
	generate_phrase(token_graph, "Finally", tokens);
	
	return 0;
}

void print_graph(array_t graph, array_t tokens)
{
	long i, j, k;
	node_t *node, *child_node;
	
	for(i = 0; i < graph.length; i++)
	{
		node = array_get_element_at(graph, i);
		printf("[");
		for(k = 0; k < NODE_NUM_PARAMS; k++)
		{
			printf("%s ", &((char *)tokens.data)[node->token[k]]);
		}
		printf("]\n");
		
		for(j = 0; j < node->children.length; j++)
		{
			child_node = array_get_element_at(node->children, j);
			printf("\t%s\n", &((char *)tokens.data)[child_node->token[0]]);
		}
	}
}

void generate_tokens(array_t* tokens, array_t* token_index)
{
		char *file_content;
		int fd;
		long i, temp_index;
		struct stat sb;
		bool first_char = true;
		char null_char = 0;
	
	
		*tokens = array_create(1000, sizeof(char));
		*token_index = array_create(1000, sizeof(long));
		
		fd = open("libro.txt", O_RDONLY);

		if(fd == -1)
		{
			perror("Error al abrir archivo\n");
			return;
		}

		if(fstat(fd, &sb) == -1)
		{
			perror("error al obtener el tamanio del archivo\n");
			close(fd);
			return;
		}

		file_content = (char *) mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

		if(file_content == MAP_FAILED)
		{
			perror("error al mapear el archivo\n");
			close(fd);
			return;
		}
		
		for(i = 0; i < sb.st_size; i++)
		{
			if(file_content[i] != 32 && file_content[i] != '\n' && file_content[i] != '\r' && file_content[i] != '.')
			{
				if(first_char)
				{
					temp_index = tokens->length;
					array_append_element(token_index, &temp_index);
					first_char = false;
				}
					
				array_append_element(tokens, &file_content[i]);
			} else {
				first_char = true;

				if(file_content[i] == '.')
				{
					array_append_element(tokens, &null_char);
					array_append_element(tokens, &file_content[i]);
					temp_index = tokens->length - 1;
					array_append_element(token_index, &temp_index);
				}

				array_append_element(tokens, &null_char);
			}
		}

		array_save_to_disk(*tokens, "model_data/tokens.arr");		
		array_save_to_disk(*token_index, "model_data/token_index.arr");
		
		munmap(file_content, sb.st_size);
		close(fd);
}

void generate_phrase(array_t word_graph, char* initial_word, array_t tokens)
{
	long random_index, *word_index;
	node_t* last_word_node = find_word(initial_word, word_graph, tokens);

	srand((unsigned int)time(NULL));

	while(last_word_node != NULL)
	{
		if(((char *)tokens.data)[last_word_node->token[0]] != '.')
		{
			printf(" ");
		}

		printf("%s", &((char *)tokens.data)[last_word_node->token[0]]);

		if(last_word_node->children.length == 0) return;

		if(((char *)tokens.data)[last_word_node->token[0]] == '.')
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

	printf("cantidad de palabras %ld\n", token_index.length);
}

node_t* find_word(char* word, array_t word_graph, array_t tokens)
{
	int i;
	node_t *element = NULL;
	char* current_word;

	for(i = 0; i < word_graph.length; i++)
	{
		element = (node_t *)array_get_element_at(word_graph, i);
		current_word =  &((char *)tokens.data)[element->token[0]];
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
	word_node.token[0] = index[0];
	array_append_element(word_graph, &word_node);
	return array_get_element_at(*word_graph, word_graph->length - 1);
}

void save_graph_to_disk(array_t word_graph)
{
	int i;
	node_t* node_temp;
	char file_name[500];

	array_save_to_disk(word_graph, "model_data/token_graph.arr");

	for( i = 0; i < word_graph.length; i++)
	{
		node_temp = array_get_element_at(word_graph, i);
		sprintf(file_name, "model_data/token_graph_%d.arr", i);
		array_save_to_disk(node_temp->children, file_name);	
	}
}

array_t build_token_graph(array_t tokens, array_t token_index)
{
	int param_counter;
	long current_word, last_word_index;
	long index[NODE_NUM_PARAMS];
	
	array_t word_graph = {0};
	node_t* last_word_node[NODE_NUM_PARAMS];

	word_graph = array_create(10, sizeof(node_t));
	
	for(current_word = 3; current_word < token_index.length; current_word++)
	{
		index[0] = *(long *)array_get_element_at(token_index, current_word);

		/* add  */
		if(current_word == 3)
		{
			word_append_to_graph(&word_graph, index);
			last_word_index = *index;
			continue;
		}
		/* find / add - add child  */
		for(param_counter = 0; param_counter < NODE_NUM_PARAMS; param_counter++)
		{
			last_word_node[param_counter] = find_word(&((char *)tokens.data)[last_word_index], word_graph, tokens);
			if(last_word_node[param_counter] == NULL)
			{
				last_word_node[param_counter] = word_append_to_graph(&word_graph, &last_word_index);
			}
			array_append_element(&last_word_node[param_counter]->children, index);
		}

		last_word_index = *index;
	}

	save_graph_to_disk(word_graph);

	return word_graph;	
}
 
