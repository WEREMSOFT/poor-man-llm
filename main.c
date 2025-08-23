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
void generate_tokens(array_t* tokens, array_t* token_index, char* training_data_filename);
void print_graph(array_t graph, array_t tokens);


void print_graph(array_t graph, array_t tokens)
{
	long i, j;
	node_t *node, *child_node;
	
	for(i = 0; i < graph.length; i++)
	{
		node = array_get_element_at(graph, i);
		printf("%s\n", &((char *)tokens.data)[node->token]);
		for(j = 0; j < node->children.length; j++)
		{
			child_node = array_get_element_at(node->children, j);
			printf("\t%s\n", &((char *)tokens.data)[child_node->token]);
		}
	}
}

void generate_tokens(array_t* tokens, array_t* token_index, char* training_data_filename)
{
		char *file_content;
		int fd;
		long i, temp_index;
		struct stat sb;
		bool first_char = true;
		char null_char = 0;
	
	
		*tokens = array_create(1000, sizeof(char));
		*token_index = array_create(1000, sizeof(long));
		
		fd = open(training_data_filename, O_RDONLY);

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
	int i;
	long current_word, *index, last_word_index;
	array_t word_graph = {0};
	node_t *last_word_node, *node_temp;
	char file_name[500] = {0};

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

	array_save_to_disk(word_graph, "model_data/token_graph.arr");

	for( i = 0; i < word_graph.length; i++)
	{
		node_temp = array_get_element_at(word_graph, i);
		sprintf(file_name, "model_data/token_graph_%d.arr", i);
		array_save_to_disk(node_temp->children, file_name);	
	}

	return word_graph;	
}

long get_dictionary_index(array_t* token_dictionary, array_t* dictionary_index,  char* token_string)
{
	long i, *dictionary_i, return_value;
	char *dictionary_token_string;
	for(i = 0; i < dictionary_index->length; i++)
	{
		dictionary_i = array_get_element_at(*dictionary_index, i);
		dictionary_token_string = array_get_element_at(*token_dictionary, *dictionary_i);

		if(strcmp(dictionary_token_string, token_string) == 0)
		{
			return *dictionary_i;
		}
		
	}
	return_value = token_dictionary->length;
	array_append_element(dictionary_index, &return_value);
	while(*token_string != '\0')
	{
		array_append_element(token_dictionary, token_string);
		token_string++;
	}
	
	array_append_element(token_dictionary, "\0");
	return return_value;
}

void generate_dictionary(array_t *token_dictionary, array_t *token_dictionary_index, array_t tokens, array_t token_index)
{
	long i, *token_i;
	char* token_string;
	
	for(i = 0; i < token_index.length; i++)
	{
		token_i = (long *)array_get_element_at(token_index, i);
		token_string = (char*)array_get_element_at(tokens, *token_i);
		get_dictionary_index(token_dictionary, token_dictionary_index,  token_string);
	}
}

void print_dictionary(array_t dictionary_token, array_t dictionary_token_index)
{
	long i, *token_i;
	char *token_string;
	for(i = 0; i < dictionary_token_index.length; i++)
	{
		token_i = (long *)array_get_element_at(dictionary_token_index, i);
		token_string = (char*)array_get_element_at(dictionary_token, *token_i);
		printf("%s\n", token_string);
	}
}

void generate_tokenized_training_data(array_t *training_data, array_t dictionary_token, array_t dictionary_token_index, array_t tokens, array_t token_index)
{
	long i, *token_i;
	char *token_string;
	long dictionary_index;
	for(i = 0; i < token_index.length; i++)
	{
		token_i = (long *)array_get_element_at(token_index, i);
		token_string = (char*)array_get_element_at(tokens, *token_i);
		dictionary_index = get_dictionary_index(&dictionary_token, &dictionary_token_index,  token_string);
		array_append_element(training_data, &dictionary_index);
	}
}

void print_tokenized_data(array_t tokenized_data, array_t dictionary)
{
	long i, *token_i;
	char *token_string;
	for(i = 0; i < tokenized_data.length; i++)
	{
		token_i = (long *)array_get_element_at(tokenized_data, i);
		token_string = (char*)array_get_element_at(dictionary, *token_i);
		printf("%ld %s | ", *token_i, token_string);
	}
	printf("\n");
}

node_n_t* get_node_by_key(array_t token_graph, long key[NODE_NUM_PARAM])
{
	long i, j;
	bool found = false;
	node_n_t *node;

	for(i = 0; i < token_graph.length && !found; i++)
	{
		node = (node_n_t*)array_get_element_at(token_graph, i);

		found = true;
		for(j = 0; j < NODE_NUM_PARAM; j++)
		{
			found &= node->key[j] == key[j];
		}
	}

	return found ? node : NULL;
}

void build_graph_from_tokenized_data(array_t *token_graph, array_t tokenized_training_data)
{
	long i, j, *training_token, parent_key[NODE_NUM_PARAM] = {0}, actual_key[NODE_NUM_PARAM] = {0};
	node_n_t actual_node = {0}, *parent_node, *actual_node_p;

	for(i = NODE_NUM_PARAM; i < tokenized_training_data.length - 1; i++)
	{
		actual_node = node_n_create();

	 	for(j = 0; j < NODE_NUM_PARAM; j++)
		{
			training_token = array_get_element_at(tokenized_training_data, i - NODE_NUM_PARAM + j);
			parent_key[j] = *training_token;
			training_token = array_get_element_at(tokenized_training_data, i + 1 - NODE_NUM_PARAM + j);
			actual_key[j] = *training_token;
			actual_node.key[j] = *training_token;
		}

		actual_node_p = get_node_by_key(*token_graph, actual_key);

		if(actual_node_p != NULL)
		{
			actual_node = *actual_node_p;
		} else {
			actual_node.index = token_graph->length;
			array_append_element(token_graph, &actual_node);
		}

		parent_node = get_node_by_key(*token_graph, parent_key);
		if(parent_node != NULL)
		{
			array_append_element(&parent_node->children, &actual_node.index);
		}
	}
}

array_t load_token_graph()
{
	long i;
	array_t token_graph;
	node_n_t *node_temp;
	char file_name[500];

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

	return token_graph;
}

void save_token_graph(array_t token_graph)
{
	node_n_t *node_temp;
	char file_name[500] = {0};
	long i;

	array_save_to_disk(token_graph, "model_data/token_graph.arr");
	for( i = 0; i < token_graph.length; i++)
	{
		node_temp = array_get_element_at(token_graph, i);
		sprintf(file_name, "model_data/token_graph_%ld.arr", i);
		array_save_to_disk(node_temp->children, file_name);	
	}
}

void print_token_graph(array_t graph, array_t tokens)
{
	long i, j, k, *child_node_index;
	node_n_t *node, *child_node;
	
	for(i = 0; i < graph.length; i++)
	{
		node = array_get_element_at(graph, i);
		
		if(node->children.length < 2)
		{
			continue;
		}

		for(k = 0; k < NODE_NUM_PARAM; k++)
		{
			printf("%s-", &((char *)tokens.data)[node->key[k]]);
		}
		printf("\n");
		for(j = 0; j < node->children.length; j++)
		{
			child_node_index = array_get_element_at(node->children, j);
			child_node = (node_n_t*)array_get_element_at(graph, *child_node_index);
			printf("\t%s\n", &((char *)tokens.data)[child_node->key[NODE_NUM_PARAM - 1]]);
		}
	}
}

int main(void)
{
	array_t token_graph = {0};
	array_t tokens = {0};	
	array_t token_index = {0};

	array_t dictionary_token = {0};
	array_t dictionary_token_index = {0};

	array_t tokenized_training_data = {0};


	dictionary_token = array_load_from_disk("model_data/dictionary_token.arr");
	tokenized_training_data = array_load_from_disk("model_data/tokenized_training_data.arr");

	if(tokenized_training_data.length == 0)
	{
		generate_tokens(&tokens, &token_index, "libro.txt");
	
		dictionary_token = array_create(100, sizeof(char));
		dictionary_token_index = array_create(100, sizeof(long));
	
		generate_dictionary(&dictionary_token, &dictionary_token_index, tokens, token_index);
	
		tokenized_training_data = array_create(100, sizeof(long));
		generate_tokenized_training_data(&tokenized_training_data, dictionary_token, dictionary_token_index, tokens, token_index);
		array_save_to_disk(dictionary_token, "model_data/dictionary_token.arr");
		array_save_to_disk(tokenized_training_data, "model_data/tokenized_training_data.arr");
	}

	token_graph = load_token_graph();

	if(token_graph.length == 0)
	{
		token_graph = array_create(100, sizeof(node_n_t));
		build_graph_from_tokenized_data(&token_graph, tokenized_training_data);
		save_token_graph(token_graph);
	}

	print_token_graph(token_graph, dictionary_token);

	return 0;
}
