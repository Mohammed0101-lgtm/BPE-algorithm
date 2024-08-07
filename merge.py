import numpy as np
from collections import defaultdict

def read_file(filename):
    with open(filename, 'r') as file:
        text = file.read()
    return text

def tokenize(text):
    tokens = [ord(c) for c in text]  # Convert characters to their Unicode code points
    return tokens

def get_pairs(tokens):
    pairs = []
    size = len(tokens) - 1
    for i in range(size):
        pairs.append((tokens[i], tokens[i + 1]))
    return pairs

def frequent(pairs):
    counts = defaultdict(int)
    for pair in pairs:
        counts[pair] += 1
    
    sorted_pairs = sorted(counts.items(), key=lambda x: x[1], reverse=True)
    sorted_dict = dict(sorted_pairs)
    pairs = list(sorted_dict.keys())
    return pairs

def merge(tokens, pair, new_token):
    merged = []
    i = 0
    size = len(tokens)
    while i < size:
        if i < size - 1 and tokens[i] == pair[0] and tokens[i + 1] == pair[1]:
            merged.append(new_token)
            i += 2
        else:
            merged.append(tokens[i])
            i += 1
    return merged

def byte_pair_encoding(text, num_merges):
    tokens = tokenize(text)
    next_token = 256  
    for _ in range(num_merges):
        pairs = get_pairs(tokens)
        common = frequent(pairs)
        top_pair = common[0]
        tokens = merge(tokens, top_pair, next_token)
        next_token += 1
    return tokens

def main():
    text = read_file("text.txt")
    num_merges = 10
    compressed = byte_pair_encoding(text, num_merges)
    print(f'Compressed tokens (as integers): {compressed}')

if __name__ == "__main__":
    main()
