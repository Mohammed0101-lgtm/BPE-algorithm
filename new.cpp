#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <stdexcept>
#include <cassert>


using token = std::vector<uint8_t>;
using pair = std::pair<token, token>;


void print_token(token& tok) {
	std::cout << '[';
	for (uint8_t element : tok) {
		std::cout << element;
	}
	
	std::cout << ']';
}


std::string getStr(const std::string& prompt);
char32_t decode(const std::vector<uint8_t>& bytes);
std::vector<uint8_t> encode(char32_t uni_char);
std::vector<token> tokenize(std::string& text);


std::map<int, pair> merges;


int main(void) {
	std::string text = getStr("Enter text : ");

	if (text.empty()) return -1;

	std::vector<token> pre_tokens;
	
	size_t len = text.length();
	for (int i = 0; i < len; i++) {
		pre_tokens.push_back(encode(text[i]));
	}

	for (token tok : pre_tokens) {
		print_token(tok);
		std::cout << std::endl;
	}

	std::vector<token> tokens = tokenize(text);

	std::cout << "merged" << std::endl;
	for (token tok : tokens) {
		print_token(tok);
		std::cout << std::endl;
	}

	float ratio = tokens.size() > 0 ? static_cast<float>(pre_tokens.size()) / tokens.size() : 0.0f;
    
	std::cout << "compression ratio : " << ratio << std::endl;
	return 0;
}


std::vector<pair> get_pairs(std::vector<token>& tokens) {
    std::vector<pair> pairs;
    size_t tokens_size = tokens.size();

    for (size_t i = 0; i < tokens_size - 1; ++i) {
        pair p;
        p.first = tokens[i];
        p.second = tokens[i + 1];
        pairs.push_back(p);
    }

    return pairs;
}


bool compare(const std::pair<pair, int>& a, const std::pair<pair, int>& b) {
	return a.second > b.second;
}


std::vector<token> merge(std::vector<token>& tokens, int new_tok, pair p) {
    std::vector<token> merged;
    size_t tokens_size = tokens.size();
    size_t i = 0;

    while (i < tokens_size) {
        if (i < tokens_size - 1 && tokens[i] == p.first && tokens[i + 1] == p.second) {
            token tok;
            tok.push_back(new_tok);
            merged.push_back(tok);
            i += 2;
        } else {
            merged.push_back(tokens[i++]);
        }
    }

    return merged;
}


std::vector<pair> frequent(const std::vector<pair>& pairs) {
	std::map<pair, size_t> counts;
	std::vector<pair> sorted;
	
	for (const pair& p : pairs) {
		counts[p]++;
	}

	std::vector<std::pair<pair, int>> vec(counts.begin(), counts.end());
	std::sort(vec.begin(), vec.end(), compare);
	
	for (const auto& entry : vec) {
		sorted.push_back(entry.first);
	}

	return sorted;
}


std::vector<token> tokenize(std::string& text) {
	std::vector<token> tokens;
	size_t len = text.length();
	
	for (int i = 0; i < len; i++) {
		token tok = encode(text[i]);
		tokens.push_back(tok);
	}

	std::vector<pair> pairs = get_pairs(tokens);
	
	pairs = frequent(pairs);
	int vocab_size = 300;
	int times = vocab_size - 256;
	std::vector<token> id = tokens;
	
	for (int i = 0; i < times; i++) {
		std::vector<pair> common = frequent(get_pairs(id));
		int idx = 256 + i;	
		id = merge(id, idx, pairs[0]);
		merges[idx] = pairs[0];
	}

	return id;
}


std::vector<uint8_t> encode(char32_t uni_char) {
     std::vector<uint8_t> utf8;
 
     if (uni_char <= 0x7F)
         utf8.push_back(static_cast<uint8_t>(uni_char));
     else if (uni_char <= 0x7FF) {
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
     } 
     else 
         throw std::runtime_error("Invalid unicode code point");
 
     return utf8;
} 


char32_t decode(const std::vector<uint8_t>& bytes) {
    char32_t unicode = 0;

    if (bytes.empty()) 
        throw std::runtime_error("Empty UTF-8 sequence");

    if ((bytes[0] & 0x80) == 0) 
        unicode = bytes[0];
    else if ((bytes[0] & 0xE0) == 0xC0) {
        if (bytes.size() < 2) throw std::runtime_error("Invalid UTF-8 sequence");
        unicode = ((bytes[0] & 0x1F) << 6) |
                  (bytes[1] & 0x3F);
    } else if ((bytes[0] & 0xF0) == 0xE0) {
        if (bytes.size() < 3) throw std::runtime_error("Invalid UTF-8 sequence");
        unicode = ((bytes[0] & 0x0F) << 12) |
                  ((bytes[1] & 0x3F) << 6) |
                  (bytes[2] & 0x3F);
    } else if ((bytes[0] & 0xF8) == 0xF0) {
        if (bytes.size() < 4) throw std::runtime_error("Invalid UTF-8 sequence");
        unicode = ((bytes[0] & 0x07) << 18) |
                  ((bytes[1] & 0x3F) << 12) |
                  ((bytes[2] & 0x3F) << 6) |
                  (bytes[3] & 0x3F);
    } 
    else 
        throw std::runtime_error("Invalid UTF-8 sequence");

    return unicode;
}


std::string getStr(const std::string& prompt) {
	std::cout << prompt;
	std::string input;
	std::getline(std::cin, input);	

	size_t len = input.length();
	assert(len > 0);

	if (len > 0 && input[len - 1] == '\n') 
		input[len - 1] = '\0';

	return input;
}




























































