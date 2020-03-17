//
// Created by cao on 2020/3/16.
//

#ifndef TEST_KNN_H
#define TEST_KNN_H

#include <vector>
#include <cstdio>
#include <queue>

class KNN {

    struct KD_Node{
        struct KD_Node* left = NULL;
        struct KD_Node* right = NULL;
        std::vector<float > vec;
        int label;
    };

    struct PQ_Node{
        float dis;
        int label;
        std::vector<float >vec;
        bool operator < (const PQ_Node& a) const {
            return dis < a.dis;
        }
        PQ_Node(float d, int l, std::vector<float>v): dis(d), label(l), vec(v){}
        PQ_Node(){}
    };

public:
    int k, n, m, c;
    KNN(int k, int d);
    void fit(std::vector< std::vector<float> > data, std::vector<int> lables);
    std::vector<int > predict(std::vector< std::vector<float> > data);

private:
    KD_Node *root = NULL;
    std::priority_queue<PQ_Node> pq;
    KD_Node* insert(KD_Node* r, std::vector<float> vec, int label, int idx);
    float cal_distance(std::vector<float> v_a, std::vector<float> v_b);
    KD_Node* create_kd_node(std::vector<float> vec, int label);
    void search_knn(KD_Node* r, std::vector<float> vec, int idx);
};


#endif //TEST_KNN_H
