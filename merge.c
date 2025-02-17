#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_CAPACITY 1000

typedef int16_t Token;

typedef struct {
    Token first;
    Token second;
} Pair;

typedef struct Node {
    Pair         pair;
    int          frequency;
    struct Node* next;
} Node;

typedef struct {
    Node** table;
    size_t capacity;
} HashMap;

HashMap* create_map(size_t capacity) {
    HashMap* map  = (HashMap*) malloc(sizeof(HashMap));
    map->table    = (Node**) calloc(capacity, sizeof(Node*));
    map->capacity = capacity;
    return map;
}

size_t hash_pair(Pair pair, size_t capacity) { return ((pair.first * 31) + pair.second) % capacity; }

void insert_map(HashMap* map, Pair pair) {
    size_t index = hash_pair(pair, map->capacity);
    Node*  node  = map->table[index];
    while (node)
    {
        if (node->pair.first == pair.first && node->pair.second == pair.second)
        {
            node->frequency++;
            return;
        }
        node = node->next;
    }
    Node* new_node      = (Node*) malloc(sizeof(Node));
    new_node->pair      = pair;
    new_node->frequency = 1;
    new_node->next      = map->table[index];
    map->table[index]   = new_node;
}

Pair most_frequent(HashMap* map, int* max_freq) {
    Pair most_frequent_pair = {-1, -1};
    *max_freq               = 0;
    for (size_t i = 0; i < map->capacity; i++)
    {
        Node* node = map->table[i];
        while (node)
        {
            if (node->frequency > *max_freq)
            {
                *max_freq          = node->frequency;
                most_frequent_pair = node->pair;
            }
            node = node->next;
        }
    }
    return most_frequent_pair;
}

void free_map(HashMap* map) {
    for (size_t i = 0; i < map->capacity; i++)
    {
        Node* node = map->table[i];
        while (node)
        {
            Node* temp = node;
            node       = node->next;
            free(temp);
        }
    }
    free(map->table);
    free(map);
}

void replace(Token* tokens, size_t* tokensSize, Pair most_frequent_pair, Token new_token) {
    size_t new_size = 0;
    for (size_t i = 0; i < *tokensSize; i++)
    {
        if (i < *tokensSize - 1 && tokens[i] == most_frequent_pair.first && tokens[i + 1] == most_frequent_pair.second)
        {
            tokens[new_size++] = new_token;
            i++;
        }
        else
        {
            tokens[new_size++] = tokens[i];
        }
    }
    *tokensSize = new_size;
}

void byte_pair_encoding(Token* tokens, size_t* tokensSize, int num_merges) {
    Token next_token = 256;
    for (int merge_count = 0; merge_count < num_merges; merge_count++)
    {
        HashMap* map = create_map(MAX_CAPACITY);
        for (size_t i = 0; i < *tokensSize - 1; i++)
        {
            Pair pair = {tokens[i], tokens[i + 1]};
            insert_map(map, pair);
        }
        int  max_freq;
        Pair most_frequent_pair = most_frequent(map, &max_freq);
        if (max_freq == 0)
        {
            free_map(map);
            break;
        }
        replace(tokens, tokensSize, most_frequent_pair, next_token++);
        free_map(map);
    }
}

void debug_tokens(Token* tokens, size_t tokensSize) {
    printf("Tokens: ");
    for (size_t i = 0; i < tokensSize; i++)
        printf("[%d] ", tokens[i]);

    printf("\n");
}

char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");

    if (!file)
        return NULL;

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char* text = (char*) malloc(size + 1);
    fread(text, 1, size, file);
    text[size] = '\0';
    fclose(file);

    return text;
}

unsigned long strToNum(const char *str) {
	if (!str) 
		return 0;
	if (str[0] == '-')
		return 0;

	int l = strlen(str);
	unsigned long fac = pow(10, l - 1); 
	unsigned long ret = 0;	
	
	for (int i = 0; i < l; i++) 
	{
		ret += fac * (str[i] - '0');
		fac /= 10;
	}

	return ret;
}

int main(int argc, char **argv) {
	const char *filename = "text.txt";
	int num_merges = 200;

	if (argc == 2)
		filename = strdup(argv[1]);
	else if (argc == 3)
	{
		filename = strdup(argv[1]);
		num_merges = strToNum(argv[2]);
	}

    char* text = read_file(filename);
    if (!text)
    {
        fprintf(stderr, "Failed to read file: %s\n", filename);
        return EXIT_FAILURE;
    }

    size_t text_length = strlen(text);
    Token* tokens      = (Token*) malloc(text_length * sizeof(Token));
    for (size_t i = 0; i < text_length; i++)
        tokens[i] = (Token) text[i];


    size_t tokensSize = text_length;

    printf("Original Tokens:\n");
    debug_tokens(tokens, tokensSize);

    byte_pair_encoding(tokens, &tokensSize, num_merges);

    printf("Tokens After Encoding:\n");
    debug_tokens(tokens, tokensSize);

    free(tokens);
    free(text);
    return EXIT_SUCCESS;
}







