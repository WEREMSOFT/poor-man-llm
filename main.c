#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define __ARRAY_T_IMPLEMENTATION__
#include "array_t/array_t.h"
#include "node_t/node_t.h"
#include "stopwatch.h"

node_t* get_node_by_key(array_t graph, int key[NODE_NUM_PARAM]);
void generate_tokens(array_t* tokens, array_t* token_indices, char* training_data_filename);
void generate_dictionary(array_t *dictionary, array_t *dictionary_indices, array_t tokens, array_t token_indices);
void generate_training_data(array_t *training_data, array_t dictionary, array_t dictionary_indices, array_t tokens, array_t token_indices);
void generate_phrase(array_t words, array_t graph, array_t dictionary, array_t dictionary_indices);
array_t build_graph_threaded(array_t tokenized_training_data, array_t tokens);
void save_graph(array_t graph);
array_t load_graph();
void print_graph(array_t graph, array_t tokens);
void print_word(array_t tokens, int index);
int compar_graph_keys(const void *a, const void *b);
int compar_graph_keys_n(const void *a, const void *b, void* _);

#define PTHREAD_NUM 16

typedef struct thread_params_t
{
	int start;
	int end;
	array_t graph;
	array_t tokenized_training_data; 
	array_t dictionary;
} thread_params_t;

typedef struct merge_thread_params_t
{
	array_t graph_1;
	array_t graph_2;
	array_t result;
} merge_thread_params_t;

/*
DEBUG
*/

array_t dic;

void build_graph_slice2(array_t *graph, array_t tokenized_training_data, int start, int end)
{
	int i, j, *training_token, parent_key[NODE_NUM_PARAM] = {0}, actual_key[NODE_NUM_PARAM] = {0};
	node_t actual_node = {0}, parent_node = {0}, *parent_node_p, *actual_node_p;

	/*
	uninitialized
	*/ 
	parent_key[0] = -1;

	for(i = start; i < end; i++)
	{
		actual_node = node_create();

		for(j = 0; j < NODE_NUM_PARAM; j++)
		{
			training_token = array_get_element_at(tokenized_training_data, i + j);
			
			if(training_token == NULL) 
				return;
			
			actual_key[j] = *training_token;
			actual_node.key[j] = *training_token;
			
			if(i > 0)
			{
				training_token = array_get_element_at(tokenized_training_data, i + j - 1);
				parent_key[j] = *training_token;
			}
		}

		actual_node_p = get_node_by_key(*graph, actual_key);

		if(actual_node_p != NULL)
		{
			actual_node = *actual_node_p;
		} else {
			array_insert_element_in_order(graph, &actual_node, compar_graph_keys_n);
		}

		parent_node_p = get_node_by_key(*graph, parent_key);
		if(parent_node_p != NULL)
		{
			array_append_element(&parent_node_p->children, &actual_node.key);
		} else if(parent_key[0] != -1)
		{
			parent_node = node_create();
			for(j = 0; j < NODE_NUM_PARAM; j++)
			{
				parent_node.key[j] = parent_key[j];
			}
			parent_node_p = &parent_node;
			parent_node_p = array_append_element(graph, parent_node_p);
			array_append_element(&parent_node_p->children, &actual_node.key);
		}
	}
}

void *build_graph_slice(void *params)
{
	thread_params_t *param = params;

	printf("start: %d, end: %d, graph_size: %d, training_data_size: %d\n", param->start, param->end, param->graph.capacity, param->tokenized_training_data.length);

	build_graph_slice2(&param->graph, param->tokenized_training_data, param->start, param->end);
	return NULL;
}

array_t graph_merge(array_t graph_1, array_t graph_2)
{
	int i, j;
	node_t *node_temp, *node_temp_2;

	printf("graph 1: %d, graph 2: %d\n", graph_1.length, graph_2.length);

	for(i = 0; i <  graph_2.length; i++)
	{
		node_temp_2 = array_get_element_at(graph_2, i);
		node_temp = get_node_by_key(graph_1, node_temp_2->key);

		if(node_temp == NULL)
		{
			node_temp = array_insert_element_in_order(&graph_1, node_temp_2, compar_graph_keys_n);
		} else {
			for(j = 0; j < node_temp_2->children.length; j++)
			{
				array_append_element(&node_temp->children, array_get_element_at(node_temp_2->children, j));
			}
		}

	}

	return graph_1;
}

void *merge_pair(void *params)
{
	merge_thread_params_t* param = params;
	param->result = graph_merge(param->graph_1, param->graph_2);
	return NULL;
}

array_t build_graph_threaded(array_t tokenized_training_data, array_t dictionary)
{
	int i, j, merged_count;
	pthread_t threads[PTHREAD_NUM];
	thread_params_t thread_params[PTHREAD_NUM] = {0};
	merge_thread_params_t merge_thread_params[PTHREAD_NUM] = {0};
	int slice_size = tokenized_training_data.length / PTHREAD_NUM;
	int reminder = tokenized_training_data.length % PTHREAD_NUM;
	array_t graph = {0};
	int start = 0;
	printf("WWWWWWWWWWWWWWWWWWWWWW\n");
	for(i = 0; i < PTHREAD_NUM; i++)
	{
		thread_params[i].start = start;
		thread_params[i].end = start + slice_size + (i < reminder ? 1 : 0);
		thread_params[i].tokenized_training_data = tokenized_training_data;
		thread_params[i].graph = array_create(10, sizeof(node_t));
		thread_params[i].dictionary = dictionary;
		start = thread_params[i].end;
		pthread_create(&threads[i], NULL, build_graph_slice, &thread_params[i]);
		
	}
	printf("WWWWWWWWWWWWWWWWWWWWWW\n");
	
	for(i = 0; i < PTHREAD_NUM; i++)
	{
		pthread_join(threads[i], NULL);
	}

	for(i = 0; i < PTHREAD_NUM; i++)
	{
		merge_thread_params[i].result = thread_params[i].graph;
	}
	
	merged_count = PTHREAD_NUM;
	while(merged_count > 1)
	{
		j = 0;
		for(i = 0; i < merged_count; i+=2)
		{
			merge_thread_params[j].graph_1 = merge_thread_params[i].result;
			merge_thread_params[j].graph_2 = merge_thread_params[i + 1].result;
			pthread_create(&threads[j], NULL, merge_pair, &merge_thread_params[j]);
			j++;
		}

		for(i = 0; i < merged_count / 2; i++)
		{
			pthread_join(threads[i], NULL);
		}

		merged_count /= 2;
	}

	graph = merge_thread_params[0].result;
	return graph;
}

int main(void)
{
	array_t graph = {0};
	array_t tokens = {0};	
	array_t token_indices = {0};

	array_t dictionary = {0};
	array_t dictionary_indices = {0};

	array_t tokenized_training_data = {0};

	array_t words = {0};

	stopwatch_wall_clock_start("LLM Training");

	dictionary = array_load_from_disk("model_data/dictionary.arr");
	tokenized_training_data = array_load_from_disk("model_data/tokenized_training_data.arr");
	dictionary_indices = array_load_from_disk("model_data/dictionary_indices.arr");
	
	if(tokenized_training_data.length == 0)
	{
		generate_tokens(&tokens, &token_indices, "libro_test.txt");
	
		dictionary = array_create(100, sizeof(char));
		dictionary_indices = array_create(100, sizeof(int));
	
		generate_dictionary(&dictionary, &dictionary_indices, tokens, token_indices);
	
		tokenized_training_data = array_create(100, sizeof(int));
		generate_training_data(&tokenized_training_data, dictionary, dictionary_indices, tokens, token_indices);
		array_save_to_disk(dictionary, "model_data/dictionary.arr");
		array_save_to_disk(tokenized_training_data, "model_data/tokenized_training_data.arr");
		array_save_to_disk(dictionary_indices, "model_data/dictionary_indices.arr");
	}
	
	dic = dictionary;

	graph = load_graph();

	if(graph.length == 0)
	{
		stopwatch_start("building graph");
		stopwatch_rdtsc_start("building graph");
		#ifdef MULTI 
			printf("multi thread\n");
			graph = build_graph_threaded(tokenized_training_data, dictionary);
		#else
			printf("single thread\n");
			graph = array_create(10, sizeof(node_t));
			build_graph_slice2(&graph, tokenized_training_data, 0, tokenized_training_data.length);
		#endif
		stopwatch_rdtsc_stop();
		stopwatch_stop();
		save_graph(graph);
	}

	print_graph(graph, dictionary);

	words = array_create(3, sizeof(char*));

	array_append_element(&words, "uno");
	array_append_element(&words, "dos");

	generate_phrase(words, graph, dictionary, dictionary_indices);
	printf("\n");

	return 0;
/*

*/

	words = array_create(3, sizeof(char*));

	array_append_element(&words, "The");
	array_append_element(&words, "old");
	
	generate_phrase(words, graph, dictionary, dictionary_indices);

	printf("\n");

	words = array_destroy(words);
	words = array_create(3, sizeof(char*));

	array_append_element(&words, "But");
	array_append_element(&words, "then");

	generate_phrase(words, graph, dictionary, dictionary_indices);

	printf("\n");

	words = array_destroy(words);
	words = array_create(3, sizeof(char*));

	array_append_element(&words, "Finally");
	generate_phrase(words, graph, dictionary, dictionary_indices);

	stopwatch_wall_clock_stop();

	return 0;
}

void print_word(array_t tokens, int index)
{
	if(index < 0) 
	{
		printf("\n");
		return;
	}
	printf("%s\n", &((char *)tokens.data)[index]);
}


void print_graph(array_t graph, array_t tokens)
{
	int i, j;
	node_t *node, *child_node;
	printf("################\n");
	printf("Graph Size %d, Capacity %d\n", graph.length, graph.capacity);

	for(i = 0; i < graph.length; i++)
	{
		node = array_get_element_at(graph, i);
		for(j = 0; j < NODE_NUM_PARAM; j++)
		{
			if(j > 0) 
			{
				printf("-");
			}
			printf("%d:%s", node->key[j], &((char *)tokens.data)[node->key[j]]);
		}
		printf("\n");
		for(j = 0; j < node->children.length; j++)
		{
			child_node = array_get_element_at(node->children, j);
			printf("\t%s\n", &((char *)tokens.data)[child_node->key[NODE_NUM_PARAM  - 1]]);
		}
	}
}

void generate_tokens(array_t* tokens, array_t* token_indices, char* training_data_filename)
{
		char *file_content;
		int fd;
		int i, temp_index;
		struct stat sb;
		bool first_char = true;
		char null_char = 0;
	
	
		*tokens = array_create(1000, sizeof(char));
		*token_indices = array_create(1000, sizeof(int));
		
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
					array_append_element(token_indices, &temp_index);
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
					array_append_element(token_indices, &temp_index);
				}

				array_append_element(tokens, &null_char);
			}
		}

		munmap(file_content, sb.st_size);
		close(fd);
}

void print_words(array_t tokens, array_t token_indices)
{
	int i, *index;
	for(i = 0; i < token_indices.length; i++)
	{
		index = (int *)array_get_element_at(token_indices, i);
		printf("%s\n", &((char *)tokens.data)[*index]);
	}

	printf("cantidad de palabras %d\n", token_indices.length);
}

node_t* find_word(char* word, array_t word_graph, array_t tokens)
{
	int i;
	node_t *element = NULL;
	char* current_word;

	for(i = 0; i < word_graph.length; i++)
	{
		element = (node_t *)array_get_element_at(word_graph, i);
		current_word =  &((char *)tokens.data)[element->key[0]];
		if(strcmp(current_word, word) == 0)
		{
			return element;
		}
	}
	
	return NULL;
}

node_t* word_append_to_graph(array_t* word_graph, int *index)
{
	node_t word_node = {0};
	word_node = node_create();
	word_node.key[0] = *index;
	array_append_element(word_graph, &word_node);
	return array_get_element_at(*word_graph, word_graph->length - 1);
}

array_t build_token_graph(array_t tokens, array_t token_indices)
{
	int i;
	int current_word, *index, last_word_index;
	array_t word_graph = {0};
	node_t *last_word_node, *node_temp;
	char file_name[500] = {0};

	word_graph = array_create(10, sizeof(node_t));
	
	for(current_word = 0; current_word < token_indices.length; current_word++)
	{
		index = (int *)array_get_element_at(token_indices, current_word);

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

	array_save_to_disk(word_graph, "model_data/graph.arr");

	for( i = 0; i < word_graph.length; i++)
	{
		node_temp = array_get_element_at(word_graph, i);
		sprintf(file_name, "model_data/token_graph_%d.arr", i);
		array_save_to_disk(node_temp->children, file_name);	
	}

	return word_graph;	
}

int get_dictionary_index(array_t* dictionary, array_t* dictionary_indices,  char* token_string)
{
	int i, *dictionary_i, return_value;
	char *dictionary_token_string;
	for(i = 0; i < dictionary_indices->length; i++)
	{
		dictionary_i = array_get_element_at(*dictionary_indices, i);
		dictionary_token_string = array_get_element_at(*dictionary, *dictionary_i);

		if(strcmp(dictionary_token_string, token_string) == 0)
		{
			return *dictionary_i;
		}
		
	}
	return_value = dictionary->length;
	array_append_element(dictionary_indices, &return_value);
	while(*token_string != '\0')
	{
		array_append_element(dictionary, token_string);
		token_string++;
	}
	
	array_append_element(dictionary, "\0");
	return return_value;
}

void generate_dictionary(array_t *dictionary, array_t *dictionary_indices, array_t tokens, array_t token_indices)
{
	int i, *token_i;
	char* token_string;
	
	for(i = 0; i < token_indices.length; i++)
	{
		token_i = (int *)array_get_element_at(token_indices, i);
		token_string = (char*)array_get_element_at(tokens, *token_i);
		get_dictionary_index(dictionary, dictionary_indices,  token_string);
	}
}

void print_dictionary(array_t dictionary, array_t dictionary_indices)
{
	int i, *token_i;
	char *token_string;
	for(i = 0; i < dictionary_indices.length; i++)
	{
		token_i = (int *)array_get_element_at(dictionary_indices, i);
		token_string = (char*)array_get_element_at(dictionary, *token_i);
		printf("%s\n", token_string);
	}
}

void generate_training_data(array_t *training_data, array_t dictionary, array_t dictionary_indices, array_t tokens, array_t token_indices)
{
	int i, *token_i;
	char *token_string;
	int dictionary_index;
	for(i = 0; i < token_indices.length; i++)
	{
		token_i = (int *)array_get_element_at(token_indices, i);
		token_string = (char*)array_get_element_at(tokens, *token_i);
		dictionary_index = get_dictionary_index(&dictionary, &dictionary_indices,  token_string);
		array_append_element(training_data, &dictionary_index);
	}
}

void print_tokenized_data(array_t tokenized_data, array_t dictionary)
{
	int i, *token_i;
	char *token_string;
	for(i = 0; i < tokenized_data.length; i++)
	{
		token_i = (int *)array_get_element_at(tokenized_data, i);
		token_string = (char*)array_get_element_at(dictionary, *token_i);
		printf("%d %s | ", *token_i, token_string);
	}
	printf("\n");
}

int compar_graph_keys_n(const void *a, const void *b, void* _)
{
	(void)_;
	return compar_graph_keys(a, b);
}

int compar_graph_keys(const void *a, const void *b)
{
	int j, result;
	node_t *node_a, *node_b;
	node_a = (node_t*)a;
	node_b = (node_t*)b;

	for(j = 0; j < NODE_NUM_PARAM; j++)
	{
		result = node_a->key[j] - node_b->key[j];
		if(result != 0)
			return result;
	}

	return result;
}

node_t* get_node_by_key(array_t graph, int key[NODE_NUM_PARAM])
{
	return array_search_element(graph, key, compar_graph_keys);
}

array_t get_nodes_by_key(array_t graph, int key[NODE_NUM_PARAM])
{
	int i, j;
	bool found = false;
	node_t *node;

	array_t nodes = array_create(10, sizeof(int));

	for(i = 0; i < graph.length; i++)
	{
		node = (node_t*)array_get_element_at(graph, i);

		found = true;
		for(j = 0; j < NODE_NUM_PARAM; j++)
		{
			found &= node->key[j] == key[j] || key[j] == -1;
		}
		if(found)
		{
			array_append_element(&nodes, &i);
		}
		
	}

	return nodes;
}

array_t load_graph()
{
	int i;
	array_t graph;
	node_t *node_temp;
	char file_name[500];

	graph = array_load_from_disk("model_data/graph.arr");

	if(graph.length != 0)
	{
		for( i = 0; i < graph.length; i++)
		{
			node_temp = array_get_element_at(graph, i);
			sprintf(file_name, "model_data/token_graph_%d.arr", i);
			node_temp->children = array_load_from_disk(file_name);
		}
	}

	return graph;
}

void save_graph(array_t graph)
{
	node_t *node_temp;
	char file_name[500] = {0};
	int i;

	array_save_to_disk(graph, "model_data/graph.arr");
	for( i = 0; i < graph.length; i++)
	{
		node_temp = array_get_element_at(graph, i);
		sprintf(file_name, "model_data/token_graph_%d.arr", i);
		array_save_to_disk(node_temp->children, file_name);	
	}
}

void print_token_graph(array_t graph, array_t tokens)
{
	int i, j, k, *child_node_index;
	node_t *node, *child_node;
	
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
			child_node = (node_t*)array_get_element_at(graph, *child_node_index);
			printf("\t%s\n", &((char *)tokens.data)[child_node->key[NODE_NUM_PARAM - 1]]);
		}
	}
}

void print_nodes_by_indexes(array_t graph, array_t tokens, array_t indices)
{
	int i, j, k, *child_node_index, *index;
	node_t *node, *child_node;
	
	for(i = 0; i < indices.length; i++)
	{
		index = array_get_element_at(indices, i);
		node = array_get_element_at(graph, *index);
		
		printf("%d. ", i);
		for(k = 0; k < NODE_NUM_PARAM; k++)
		{
			printf("%d %s-", node->key[k],&((char *)tokens.data)[node->key[k]]);
		}
		printf("\n");
		for(j = 0; j < node->children.length; j++)
		{
			child_node_index = array_get_element_at(node->children, j);
			child_node = (node_t*)array_get_element_at(graph, *child_node_index);
			printf("\t%s\n", &((char *)tokens.data)[child_node->key[NODE_NUM_PARAM - 1]]);
		}
	}
}

void generate_phrase(array_t words, array_t graph, array_t dictionary, array_t dictionary_indices)
{
	int i, keys[NODE_NUM_PARAM] = {0}, random_index, *index;
	array_t posible_initial_nodes = {0};
	node_t *actual_node;
	bool should_continue = true;
	char *word;

	srand((unsigned int)time(NULL));

	for(i = 0; i < NODE_NUM_PARAM; i++)
	{
		keys[i] = -1;
	}

	for(i = 0; i < NODE_NUM_PARAM && i < words.length; i++)
	{
		keys[i] = get_dictionary_index(&dictionary, &dictionary_indices, array_get_element_at(words, i));
	}
	
	posible_initial_nodes = get_nodes_by_key(graph, keys);

	random_index = rand() % posible_initial_nodes.length;

	index = array_get_element_at(posible_initial_nodes, random_index);

	actual_node = array_get_element_at(graph, *index);

	for(i = 0; i < NODE_NUM_PARAM; i++)
	{
		word = &((char *)dictionary.data)[actual_node->key[i]];
		printf("%s ", word);
		fflush(stdout);
	}

	while(should_continue)
	{
		random_index = rand() % actual_node->children.length;
		index = array_get_element_at(actual_node->children, random_index);
		actual_node = get_node_by_key(graph, index);
		word = &((char *)dictionary.data)[actual_node->key[NODE_NUM_PARAM - 1]];
		if(strcmp(word, ".") == 0)
		{
			should_continue = false;
		}
		printf("%s ", word);
		fflush(stdout);
	}
	printf("\n");
}
