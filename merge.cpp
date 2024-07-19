/*--include cpp standard librearies--*/
#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <stdexcept>

// define token tyoe as a vector of unisgned 8 bit integers
using token = std::vector<uint8_t>;

/*--prototypes--*/
std::string getStr(const std::string& prompt);
std::vector<uint8_t> encode(char32_t uni_char);
char32_t decode(const std::vector<uint8_t>& bytes);
std::vector<std::pair<token, token>> get_pairs(std::vector<token>& tokens);
void print_token(token& t);
std::vector<std::pair<token, token>> frequent
    (const std::vector<std::pair<token, token>>& pairs);
std::vector<token> merge (std::vector<token>& tokens, int new_tok,
                          std::pair<token, token> pair);

std::vector<token> tokenize(std::string text);

std::map<int, std::pair<token, token>> merges;

// main function included for testing as usual
int main() {
    // prompt the user for a string of characters
    std::string text = getStr("Enter text: ");
    if (text.empty()) {
        return -1;
    }

    // get the utf-8 encoding of the text
    // this return a stream of tokens where
    // each integer corresponds to a character
    std::vector<token> pre_tokens;
    for (int i = 0; i < text.length(); i++) {
        pre_tokens.push_back(encode(text[i]));
    }

    // print the preprocessed tokens
    for (token tok : pre_tokens) {
        print_token(tok);
        std::cout << std::endl;
    }

    // tokenize the text using BPE algorithm
    std::vector<token> tokens = tokenize(text);

    // print tokens
    std::cout << "merged" << '\n';
    for (token tok : tokens) {
        print_token(tok);
        std::cout << std::endl;
    }

    // calculate and print the compression ratio for reference
    float ratio = static_cast<float>(pre_tokens.size()) / static_cast<float>(tokens.size());      
    std::cout << "compression ratio : " << ratio << std::endl; 
    
    return 0;
}

// tokenise text using the byte pair encoding
std::vector<token> tokenize(std::string text) {
    // initialize a tokens array by utf-8 encoding 
    // each character in the text
    std::vector<token> tokens;
    int len = text.length();
    for (int i = 0; i < len; i++) {
        token tok = encode(text[i]);
        tokens.push_back(tok);
    }

    // get all possible adjacent pairs in the text
    std::vector<std::pair<token, token>> pairs = get_pairs(tokens);
    // sort pairs based on the frequency of distinct pairs
    pairs = frequent(pairs);

    int vocab_size = 300; // you can change this according to the specific requirements of the program
    int times = vocab_size - 256; // the number of merging times
    // copy tokens array
    std::vector<token> id = tokens;

    for (int i = 0; i < times; i++) {
        // get updated most common pair
        std::vector<std::pair<token, token>> common = frequent(get_pairs(id));
        int idx = 256 + i; // create new token id
        id = merge(id, idx, pairs[0]); // create a new token
        merges[idx] = pairs[0]; // store token
    }

    return id;
}

// merge the pair into one token
std::vector<token> merge (std::vector<token>& tokens,  int new_tok,
                          std::pair<token, token> pair) {
    
    std::vector<token> merged;
    int size = tokens.size(), i = 0;

    while (i < size - 1) {
        // if the given pair is found
        if (tokens[i] == pair.first && tokens[i + 1] == pair.second) {
            std::vector<uint8_t> t;
            t.push_back(new_tok); // create new token for this pair
            merged.push_back(t); // add token to list
            i += 2; // skip this pair
        } else {
            // add the current token if the pair not found
            merged.push_back(tokens[i]);
            i++;
        }
    }

    return merged;
}

bool valueComparator(const std::pair<std::pair<token, token>, int>& a,
                     const std::pair<std::pair<token, token>, int>& b) {
    return a.second > b.second; 
}

// This functions sorts the pairs list in ascending 
// order based on the frequncy of the pairs
std::vector<std::pair<token, token>> frequent
    (const std::vector<std::pair<token, token>>& pairs) {
    
    std::map<std::pair<token, token>, int> counts; // frequency map 
    std::vector<std::pair<token, token>> sorted_pairs; 

    // count frequency of each pair
    for (const auto& pair : pairs) {
        counts[pair]++;
    }

    // convert the map to a vector and sort the vector
    // sorting the map would be based on the key of each entry which is the pair 
    // while we want to sort the pair based on the value of each entry
    std::vector<std::pair<std::pair<token, token>, int>> vec(counts.begin(), counts.end());
    std::sort(vec.begin(), vec.end(), valueComparator);

    // store the pairs in the result vector
    for (const auto& entry : vec) {
        sorted_pairs.push_back(entry.first);
    }

    return sorted_pairs;
}

// this just returns the set of all possible pairs
std::vector<std::pair<token, token>> get_pairs(std::vector<token>& tokens) {
    std::vector<std::pair<token, token>> pairs;
    int size = tokens.size(), i = 0; 
    while (i < size - 1) {
        std::pair<token, token> p;
        p.first = tokens[i];
        p.second = tokens[i + 1];
        pairs.push_back(p);
        i++;
    }

    return pairs;
}

// utf-8 encoder (turn a 32 bit char into an 8 bit integer)
std::vector<uint8_t> encode(char32_t uni_char) {
    // we use a vector instead of just an int
    // to account for multiple byte representations 
    // of any unicode code point 
    std::vector<uint8_t> utf8;

    if (uni_char <= 0x7F) {
        utf8.push_back(static_cast<uint8_t>(uni_char));
    } else if (uni_char <= 0x7FF) {
        utf8.push_back(static_cast<uint8_t>((uni_char >> 6) | 0xC0));
        utf8.push_back(static_cast<uint8_t>((uni_char & 0x3F) | 0x80));
    } else if (uni_char <= 0xFFFF) {
        utf8.push_back(static_cast<uint8_t>((uni_char >> 12) | 0xE0));
        utf8.push_back(static_cast<uint8_t>(((uni_char >> 6) & 0x3F) | 0x80));
        utf8.push_back(static_cast<uint8_t>((uni_char & 0x3F) | 0x80));
    } else if (uni_char <= 0x10FFFF) {
        utf8.push_back(static_cast<uint8_t>((uni_char >> 18) | 0xF0));
        utf8.push_back(static_cast<uint8_t>(((uni_char >> 12) & 0x3F) | 0x80));
        utf8.push_back(static_cast<uint8_t>(((uni_char >> 6) & 0x3F) | 0x80));
        utf8.push_back(static_cast<uint8_t>((uni_char & 0x3F) | 0x80));
    } else {
        throw std::runtime_error("Invalid unicode code point");
    }

    return utf8;
}

// utf-8 decoder
char32_t decode(const std::vector<uint8_t>& bytes) {
    char32_t unicode = 0;

    if (bytes.empty()) {
        throw std::runtime_error("Empty UTF-8 sequence");
    }

    if ((bytes[0] & 0x80) == 0) {
        unicode = bytes[0];
    } else if ((bytes[0] & 0xE0) == 0xC0) {
        if (bytes.size() < 2) throw std::runtime_error("Invalid UTF-8 sequence");
        unicode = ((bytes[0] & 0x1F) << 6) |
                  (bytes[1] & 0x3F);
    } else if ((bytes[0] & 0xF0) == 0xE0) {
        // 3-byte sequence
        if (bytes.size() < 3) throw std::runtime_error("Invalid UTF-8 sequence");
        unicode = ((bytes[0] & 0x0F) << 12) |
                  ((bytes[1] & 0x3F) << 6) |
                  (bytes[2] & 0x3F);
    } else if ((bytes[0] & 0xF8) == 0xF0) {
        // 4-byte sequence
        if (bytes.size() < 4) throw std::runtime_error("Invalid UTF-8 sequence");
        unicode = ((bytes[0] & 0x07) << 18) |
                  ((bytes[1] & 0x3F) << 12) |
                  ((bytes[2] & 0x3F) << 6) |
                  (bytes[3] & 0x3F);
    } else {
        throw std::runtime_error("Invalid UTF-8 sequence");
    }

    return unicode;
}

// prompt the user for a string and return it
std::string getStr(const std::string& prompt) {
    std::cout << prompt;
    std::string s;
    std::getline(std::cin, s);
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return s;
}
