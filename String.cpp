//
// Created by cao on 2020/3/18.
//

#include "String.h"

void String::build_trie(std::vector<std::string> words) {
    root = new Trie_Node();
    int word_sz = words.size();
    for (int i = 0; i < word_sz; ++i) {
        insert_trie(words[i]);
    }
}

int String::search_trie(std::string prefix_string) {
    int w_len = prefix_string.size();
    Trie_Node* tn = root;
    for (int i = 0; i < w_len; ++i) {
        int idx = prefix_string[i] - 'a';
        if(!tn->words[idx]) return -1;
        tn = tn->words[idx];
    }
    return tn->cnt;
}

void String::insert_trie(std::string w) {
    int w_len = w.size();
    Trie_Node* tn = root;
    for (int i = 0; i < w_len; ++i) {
        int idx = w[i] - 'a';
        if(!tn->words[idx]) tn->words[idx] = new Trie_Node();
        tn = tn->words[idx];
        tn->cnt += 1;
    }
    tn->end += 1;
}
