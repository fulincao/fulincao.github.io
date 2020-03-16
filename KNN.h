//
// Created by cao on 2020/3/16.
//

#ifndef TEST_KNN_H
#define TEST_KNN_H

#include <vector>
#include <cstdio>

class KNN {

    typedef struct Node{
        struct Node* left = NULL;
        struct Node* right = NULL;
        std::vector<float > data;
    }KD_Node;

public:
    int k;
    KNN(int k);
    void fit(std::vector< std::vector<float> > data, std::vector<int> lables);
    std::vector<int > predict(std::vector< std::vector<float> > data);

private:
    KD_Node *root = NULL;
    void insert();
};


#endif //TEST_KNN_H
