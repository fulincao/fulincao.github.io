//
// Created by cao on 2020/3/18.
//

#ifndef TEST_STRING_H
#define TEST_STRING_H

#include <vector>
#include <string>

class String {

    struct Trie_Node {
        Trie_Node* words[26];
        int cnt;
        int end;
        Trie_Node(){
            for (int i = 0; i < 26; ++i) {
                words[i] = NULL;
            }
            cnt = 0;
            end = 0;
        }
    };

public:
    void build_trie(std::vector<std::string> words);
    int search_trie(std::string prefix_string);
    void insert_trie(std::string w);
    void init_kmp(std::string pattern);
    int search_kmp(std::string text);


private:
    Trie_Node* root = NULL;
    std::vector<int> next;
    std::string kmp_pattern;

};


#endif //TEST_STRING_H
