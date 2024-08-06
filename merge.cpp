#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <fstream>
#include <cassert>

using token = uint8_t;
using pair = std::pair<token, token>;

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};

void print_token(token tok) {
    std::cout << '[' << static_cast<int>(tok) << ']';
}

std::string read_file(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file.is_open()) 
        throw std::runtime_error("Failed to open file");

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    return content;
}

token encode(char c) {
    return static_cast<token>(static_cast<unsigned char>(c));
}

std::vector<token> tokenize(const std::string& text) {
    std::vector<token> tokens;
    
    for (char c : text) 
        tokens.push_back(encode(c));

    return tokens;
}

std::vector<pair> get_pairs(const std::vector<token>& tokens) {
    std::vector<pair> pairs;

    for (size_t i = 0; i < tokens.size() - 1; ++i) 
        pairs.emplace_back(tokens[i], tokens[i + 1]);
    
    return pairs;
}

std::vector<pair> frequent(const std::vector<pair>& pairs) {
    std::unordered_map<pair, int, pair_hash> counts;
  
    for (const auto& p : pairs) 
        counts[p]++;  

    std::vector<pair> sorted;
    std::vector<std::pair<pair, int>> vec(counts.begin(), counts.end());
   
    std::sort(vec.begin(), vec.end(), [](const auto& a, const auto& b) 
                                        {return a.second > b.second;});

    for (const auto& entry : vec) 
        sorted.push_back(entry.first);

    return sorted;
}

std::vector<token> merge(const std::vector<token>& tokens, token new_tok, pair p) {
    std::vector<token> merged;
    size_t i = 0;

    while (i < tokens.size()) 
        if (i < tokens.size() - 1 && tokens[i] == p.first && tokens[i + 1] == p.second) {
            merged.push_back(new_tok);
            i += 2;
        } else {
            merged.push_back(tokens[i++]);
        }

    return merged;
}

std::vector<token> byte_pair_encoding(const std::string& text, int num_merges) {
    std::vector<token> tokens = tokenize(text);
    int next_token = 256; 

    for (int i = 0; i < num_merges; ++i) {
        std::vector<pair> pairs = get_pairs(tokens);
        std::vector<pair> common = frequent(pairs);

        if (common.empty()) break;

        pair top_pair = common[0];
        tokens = merge(tokens, next_token++, top_pair);
    }

    return tokens;
}

int main() {
    try {
        std::string text = read_file("text.txt");
        int num_merges = 10; 

        std::vector<token> compressed_tokens = byte_pair_encoding(text, num_merges);

        std::cout << "Compressed tokens:" << std::endl;
        for (token tok : compressed_tokens) {
            print_token(tok);
            std::cout << ' ';
        }
        
        std::cout << std::endl;
        
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}