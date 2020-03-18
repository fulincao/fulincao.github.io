//
// Created by cao on 2020/3/18.
//

#include <cstdio>
#include "String.h"

void String::build_trie(std::vector<std::string> words) {
    root = new Trie_Node();
    int word_sz = words.size();
    for (int i = 0; i < word_sz; ++i) {
        insert_trie(words[i]);
    }
}

int String::search_trie(std::string prefix_string) {
    if(!root) {
        printf("not build\n");
        return -1;
    }
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

void String::init_kmp(std::string pattern) {
    kmp_pattern = pattern;
    int pattern_sz = pattern.size();
    next.resize(pattern_sz);
    next[0] = -1;
    int j = 0, k = -1;
    while(j < pattern_sz) {
        if(k == -1 || pattern[j] == pattern[k]) {
            j++;
            k++;
            if(pattern[j] == pattern[k]) next[j] = next[k];
            else next[j] = k;
        }
        else k = next[k];
    }
}

int String::search_kmp(std::string text) {
    int text_sz = text.size();
    int j = 0, k = 0, pattern_sz = next.size();
    int cnt = 0;
    while( k < text_sz) {
        if(j == -1 || kmp_pattern[j] == text[k]) {
            j++;
            k++;
        }else j = next[j];
        if(j == pattern_sz) {
            j = 0;
            cnt += 1;
        }
    }
    return cnt;
}
