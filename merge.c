#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef __uint8_t token;

struct pair {
    token first;
    token second;
};

struct entry {
    struct pair key;
    int         value;

    struct entry* next;
};

struct map {
    struct entry** entries;
    size_t         size;
    size_t         capacity;
};

void print_token(token* tok) { printf("[%i]", *tok); }


/*We will just assume that no errors come out*/
char* read_file(const char* __restrict filename) {
    // open file
    FILE* fp = fopen(filename, "r");
    if (!fp)
    {
        fprintf(stderr, "Failed to open file\n");
        return NULL;
    }

    // get file size
    fseek(fp, 0, SEEK_END);
    size_t buffer_size = ftell(fp);
    rewind(fp);

    // allocate memory for the input buffer
    char* buf = (char*) malloc((buffer_size + 1) * sizeof(char));
    if (!buf)
    {
        fprintf(stderr, "Failed to allocate memory!\n");
        fclose(fp);
        return NULL;
    }

    // read file into the input buffer
    if (fread(buf, sizeof(char), buffer_size, fp) != buffer_size)
    {
        fprintf(stderr, "Failed to read file : %s\n", filename);
        free(buf);
        fclose(fp);
        return NULL;
    }

    buf[buffer_size] = '\0';  // nul terminate
    fclose(fp);               // close file

    return buf;
}

/* hashing function for the hash map */
uint64_t hash_pair(const struct pair* p) {
    uint64_t hash = 5381;

    // We will just hash both elements sequentially
    hash = ((hash << 5) + hash) ^ (uint64_t) p->first;
    hash = ((hash << 5) + hash) ^ (uint64_t) p->second;

    return hash;
}

/*create a hash map*/
struct map* create_map(int capacity) {
    // allocate memory for the map pointer
    struct map* m = (struct map*) malloc(sizeof(struct map));
    if (!m)
    {
        fprintf(stderr, "Failed to allocate memory\n");
        return NULL;
    }

    // init map
    m->size     = 0;
    m->capacity = capacity;

    // allocate memory for the entries in the map based on the capacity
    m->entries = (struct entry**) calloc(capacity, sizeof(struct entry));
    if (!m->entries)
    {
        fprintf(stderr, "Failed to allocate memory\n");
        free(m);
        return NULL;
    }

    return m;
}

/* get a value of a key in the map (the key is the pair)*/
int get(struct map* m, const struct pair* key) {
    int16_t index = hash_pair(key);  // get the presumed index of the key
    index         = index % m->capacity;
    assert(index >= 0 && index < m->capacity);  // just in case
    struct entry* e = m->entries[index];        // get the entry in which the key might exist

    // walk the entry and look for the key
    while (e)
    {
        if (key->first == e->key.first && key->second == e->key.second)
            return e->value;  // return value if key is found

        e = e->next;
    }

    return 0;
}

/*add a new entry to the map*/
void put(struct map* m, struct pair key) {
    // calculaye the index of the entry
    int16_t       index = hash_pair(&key) % m->capacity;
    struct entry* e     = m->entries[index];  // get the entry for the key

    // walk the entry and if the key is found there then just increment the value
    // that just works perfectly for our use case
    while (e)
    {
        if (e->key.first == key.first && e->key.second == key.second)
        {
            e->value++;
            return;
        }

        e = e->next;
    }

    // well if the key is not found in the map , just make a new entry for it
    struct entry* new_entry = (struct entry*) malloc(sizeof(struct entry));
    if (!new_entry)
    {
        fprintf(stderr, "Failed to allocate memory\n");
        return;
    }

    new_entry->value      = 1;
    new_entry->key.first  = key.first;
    new_entry->key.second = key.second;
    new_entry->next       = m->entries[index];
    m->entries[index]     = new_entry;
    m->size++;
}

// free the array of pairs
void free_pair_array(struct pair* pairs) {
    if (pairs)
        free(pairs);
}

/*free the map structure*/
void free_map(struct map* m) {
    if (m)
    {
        for (size_t i = 0; i < m->capacity; i++)
        {
            struct entry* e = m->entries[i];

            while (e)
            {
                struct entry* next = e->next;
                free(e);
                e = next;
            }
        }

        free(m->entries);
        free(m);
    }
}

/*encode a character to an int*/
token encode(char c) { return (token) ((unsigned char) c); }

/*turns text into tokens of integers to make it more straight forward*/
token* tokenize(const char* __restrict text) {
    if (!text)
        return NULL;

    size_t len    = strlen(text);
    token* tokens = (token*) malloc(len * sizeof(token));

    if (!tokens)
    {
        fprintf(stderr, "Failed to allocate memory\n");
        return NULL;
    }

    for (int i = 0; i < len; i++)
    {
        if (isalpha(text[i]) || isspace(text[i]))
            tokens[i] = encode(text[i]);
    }

    return tokens;
}

/*get pairs of tokens present in text (pairs are always a subset)*/
struct pair* get_pair(const token* tokens, size_t tokens_size) {
    struct pair* pairs = (struct pair*) malloc((tokens_size - 1) * sizeof(struct pair));

    if (!pairs)
    {
        fprintf(stderr, "Failed to allocate memory\n");
        return NULL;
    }

    for (int i = 0, k = 0; i < tokens_size - 1; i++, k++)
    {
        pairs[k].first  = tokens[i];
        pairs[k].second = tokens[i + 1];
    }

    return pairs;
}

/*insert an element to an array*/
void insert(struct entry e, struct entry** array, size_t size) {
    int i = 0;
    for (; i < size; i++)
    {
        if (e.value < array[i]->value)
            break;
    }

    for (int j = size; j > i; j--)
        array[j] = array[j - 1];

    array[i] = &e;
}

struct pair* frequent(const struct pair* pairs, size_t pairs_size) {
    struct map* counts = create_map(pairs_size);
    if (!counts)
        return NULL;

    for (int i = 0; i < pairs_size; i++)
        put(counts, pairs[i]);

    struct entry** vec = (struct entry**) malloc(counts->size * sizeof(struct entry*));
    if (!vec)
    {
        fprintf(stderr, "Failed to allocate memory\n");
        free_map(counts);
        return NULL;
    }

    int k = 0;
    for (int i = 0; i < counts->capacity; i++)
    {
        struct entry* e = counts->entries[i];

        while (e)
        {
            vec[k++] = e;
            e        = e->next;
        }
    }

    for (int i = 0; i < k - 1; i++)
    {
        for (int j = i + 1; j < k; j++)
        {
            if (vec[j]->value > vec[i]->value)
            {
                struct entry* temp = vec[i];
                vec[i]             = vec[j];
                vec[j]             = temp;
            }
        }
    }

    struct pair* sorted = (struct pair*) malloc(pairs_size * sizeof(struct pair));
    if (!sorted)
    {
        fprintf(stderr, "Failed to allocate memory\n");
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

token* merge(const token* tokens, size_t tokens_size, token new_token, struct pair p, size_t* returnSize) {
    token* merged = (token*) malloc(tokens_size * sizeof(token));
    if (!merged)
    {
        fprintf(stderr, "Failed to allocate memory\n");
        return NULL;
    }

    size_t i = 0, k = 0;

    while (i < tokens_size)
    {
        if (i < tokens_size - 1 && tokens[i] == p.first && tokens[i + 1] == p.second)
        {
            merged[k++] = new_token;
            i += 2;
        }
        else
        {
            merged[k++] = tokens[i++];
        }
    }

    *returnSize = k;
    return merged;
}

token* byte_pair_encode(const char* text, int num_merges, size_t* returnSize) {
    token* tokens = tokenize(text);
    if (!tokens)
        return NULL;

    int    next                 = 256;
    size_t returned_tokens_size = strlen(text);

    struct pair* pairs  = NULL;
    struct pair* common = NULL;

    for (int i = 0; i < num_merges; i++)
    {
        pairs  = get_pair(tokens, strlen(text));
        common = frequent(pairs, strlen(text) - 1);
        if (!pairs || !common)
        {
            free(tokens);
            return NULL;
        }

        struct pair top_pair   = common[0];
        size_t      new_size   = 0;
        token*      new_tokens = merge(tokens, returned_tokens_size, next++, top_pair, &new_size);

        if (!new_tokens)
        {
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
    char*  text        = read_file("text.txt");
    int    num_merges  = 10;
    size_t tokens_size = 0;
    token* tokens      = byte_pair_encode(text, num_merges, &tokens_size);

    if (!tokens)
        return -1;

    for (int i = 0; i < tokens_size; i++)
    {
        print_token(&tokens[i]);
        printf(" ");
    }

    printf("\n");
    free(tokens);

    return 0;
}
