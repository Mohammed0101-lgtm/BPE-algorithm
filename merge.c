#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

typedef uint8_t token;

struct pair {
    token first;
    token second;
};

struct entry {
    struct pair key;
    int value;
    struct entry *next;
};

struct map {
    struct entry **entries;
    size_t size;
    size_t capacity;
};

void print_token(token *tok) { 
    printf("[%i]", *tok); 
}

char *read_file(const char *__restrict filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, 
                "Failed to open file\n");
       
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    size_t buffer_size = ftell(fp);
    rewind(fp);

    char *buf = (char*)malloc((buffer_size + 1) * sizeof(char)); 
    if (!buf) {
        fprintf(stderr,
                "Failed to allocate memory!\n");
        
        fclose(fp);
        return NULL;
    } 

    if (fread(buf, sizeof(char), buffer_size, fp) != buffer_size) {
        fprintf(stderr, 
                "Failed to read file : %s\n", filename);
        
        free(buf);
        fclose(fp);
        return NULL;
    }

    buf[buffer_size] = '\0'; 

    fclose(fp);

    return buf;
}


uint64_t hash_pair(const struct pair* pair) {
    uint64_t hash = 5381;

    hash = ((hash << 5) + hash) ^ (uint64_t)pair->first;
    hash = ((hash << 5) + hash) ^ (uint64_t)pair->second;
    
    return hash;
}

struct map *create_map(int capacity) {
    struct map* m = (struct map*)malloc(sizeof(struct map));
    if (!m) {
        fprintf(stderr, 
                "Failed to allocate memory\n");

        return NULL;
    }

    m->size     = 0;
    m->capacity = capacity;
    m->entries  = (struct entry**)calloc(capacity, sizeof(struct entry));
    
    if (!m->entries) {
        fprintf(stderr, 
                "Failed to allocate memory\n");

        free(m);
        return NULL;
    }

    return m;
}

int get(struct map *m, const struct pair *key) {
    int16_t index = hash_pair(key);
    index         = index % m->capacity;
    assert(index >= 0 && index < m->capacity);
    struct entry *e = m->entries[index];

    while (e) {
        if (key->first == e->key.first && key->second == e->key.second) 
            return e->value;

        e = e->next;
    }

    return 0;
}

void put(struct map *map, struct pair key) {
    int16_t index = hash_pair(&key) % map->capacity;
    struct entry *e = map->entries[index];

    while (e) {
        if (e->key.first == key.first && e->key.second == key.second) {
            e->value++;  
            return;
        }

        e = e->next;
    }

    struct entry *new_entry = (struct entry*)malloc(sizeof(struct entry));
    if (!new_entry) {
        fprintf(stderr, "Failed to allocate memory\n");
        return;
    }

    new_entry->value      = 1; 
    new_entry->key.first  = key.first;
    new_entry->key.second = key.second;
    new_entry->next       = map->entries[index];
    map->entries[index]   = new_entry;  
    map->size++; 
}

void free_pair_array(struct pair *pairs) {
    if (pairs) {
        free(pairs);
    }
}

void free_map(struct map *m) {
    if (m) {
        for (size_t i = 0; i < m->capacity; i++) {
            struct entry *e = m->entries[i];
     
            while (e) {
                struct entry *next = e->next;
                free(e);
                e = next;
            }
        }
     
        free(m->entries);
        free(m);
    }
}

token encode(char c) {
    return (token)((unsigned char)c);
}

token *tokenize(const char *__restrict text) {
    if (!text) 
        return NULL;

    size_t len    = strlen(text);
    token *tokens = (token*)malloc(len * sizeof(token));
    
    if (!tokens) {
        fprintf(stderr, 
                "Failed to allocate memory\n");

        return NULL;
    }

    for (int i = 0; i < len; i++) 
        if (isalpha(text[i]) || isspace(text[i])) 
            tokens[i] = encode(text[i]);

    return tokens;
}

struct pair *get_pair(const token* tokens, size_t tokens_size) {
    struct pair *pairs = (struct pair*)malloc((tokens_size - 1) * sizeof(struct pair));
    
    if (!pairs) {
        fprintf(stderr, 
                "Failed to allocate memory\n");
        
        return NULL;
    }

    for (int i = 0, k = 0; i < tokens_size - 1;  i++, k++) {
        pairs[k].first  = tokens[i];
        pairs[k].second = tokens[i + 1];
    }

    return pairs;
}


void insert(struct entry entry, struct entry **array, size_t size) {
    int i = 0;

    for (; i < size; i++) 
        if (entry.value < array[i]->value) 
            break;

    for (int j = size; j > i; j--) 
        array[j] = array[j - 1];
    
    array[i] = &entry;
}

struct pair *frequent(const struct pair *pairs, size_t pairs_size) {
    struct map *counts = create_map(pairs_size);
    if (!counts)
        return NULL;

    for (int i = 0; i < pairs_size; i++) 
        put(counts, pairs[i]);

    struct entry **vec = (struct entry**)malloc(counts->size * sizeof(struct entry*));
    if (!vec) {
        fprintf(stderr, 
                "Failed to allocate memory\n");
        
        free_map(counts);
        return NULL;
    } 

    int k = 0;
    for (int i = 0; i < counts->capacity; i++) {
        struct entry *e = counts->entries[i];
     
        while (e) {
            vec[k++] = e;
            e        = e->next;
        }
    }

    for (int i = 0; i < k - 1; i++) 
        for (int j = i + 1; j < k; j++) 
            if (vec[j]->value > vec[i]->value) {
                struct entry *temp = vec[i];
                vec[i]             = vec[j];
                vec[j]             = temp;
            }


    struct pair *sorted = (struct pair*)malloc(pairs_size * sizeof(struct pair));
    if (!sorted) {
        fprintf(stderr, 
                "Failed to allocate memory\n");
        
        free(vec);
        free_map(counts);
        return NULL;
    }

    for (int i = 0; i < k; i++)
        sorted[i] = vec[i]->key;

    free(vec);
    free_map(counts);
    
    return sorted;
}


token *merge(const token *tokens, size_t tokens_size, token new_token, struct pair p, size_t *returnSize) {
    token *merged = (token*)malloc(tokens_size * sizeof(token));
    if (!merged) {
        fprintf(stderr, 
                "Failed to allocate memory\n");
        
        return NULL;
    } 

    size_t i = 0, k = 0;

    while (i < tokens_size) {
        if (i < tokens_size - 1 && tokens[i] == p.first && tokens[i + 1] == p.second) {
            merged[k++] = new_token;
            i += 2;
        } else {
            merged[k++] = tokens[i++];
        }
    }
    *returnSize = k;
    return merged;
}


token *byte_pair_encode(const char *text, int num_merges, size_t *returnSize) {
    token *tokens = tokenize(text);
    if (!tokens) 
        return NULL;
    
    int next                    = 256;
    size_t returned_tokens_size = strlen(text);

    struct pair *pairs  = NULL;
    struct pair *common = NULL;

    for (int i = 0; i < num_merges; i++) {
        pairs  = get_pair(tokens, strlen(text));
        common = frequent(pairs, strlen(text) - 1);
        if (!pairs || !common) {
            free(tokens);
            return NULL;
        }

        struct pair top_pair = common[0];
        size_t new_size      = 0;
        token *new_tokens    = merge(tokens, returned_tokens_size, next++, top_pair, &new_size);
        
        if (!new_tokens) {
            free(pairs);
            free(common);
            free(tokens);
            return NULL;
        }
        
        tokens               = new_tokens;
        returned_tokens_size = new_size;
        
        free(pairs);
        free(common);
    }

    *returnSize = returned_tokens_size;
    return tokens;
}


int main(void) {
    char *text         = read_file("text.txt");
    int num_merges     = 10;
    size_t tokens_size = 0;
    token *tokens      = byte_pair_encode(text, num_merges, &tokens_size);
    
    if (!tokens) 
        return -1;

    for (int i = 0; i < tokens_size; i++) {
        print_token(&tokens[i]);
        printf(" ");
    }

    printf("\n");
    free(tokens);
    
    return 0;
}
