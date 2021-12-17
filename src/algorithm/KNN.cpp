//
// Created by cao on 2020/3/16.
//

#include <cassert>
#include "KNN.h"
#include <cstdio>
#include <cstring>

void KNN::fit(std::vector<std::vector<float> > data, std::vector<int> lables) {
    assert(data.size() == lables.size());
    int data_len = data.size();
    KNN::n = data_len;
    KNN::m = data[0].size();
    for(int i = 0; i < data_len; i++){
        KNN::root = insert(KNN::root, data[i], lables[i], 0);
    }
}

std::vector<int> KNN::predict(std::vector<std::vector<float> > data) {

    std::vector<int> res;
    if(!KNN::root) {
        printf("not fit");
        return res;
    }
    int data_len = data.size();
    int labels[KNN::c];
    for(int i = 0; i < data_len; i++ ){
        KNN::search_knn(KNN::root, data[i], 0);
        memset(labels, 0, sizeof(labels));
        while(!KNN::pq.empty()) {
            KNN::PQ_Node now = KNN::pq.top();
            KNN::pq.pop();
            labels[now.label] += 1;
            // printf("%d, %f\n", now.label, now.dis);
        }
        int mx = -1, l = -1;
        for(int j = 0; j < KNN::c; j++) {
            // printf("labels, %d %d\n", j, labels[j]);
            if(labels[j] > mx) mx = labels[j], l = j;
        }
        res.push_back(l);
    }
    return res;
}

KNN::KNN(int k, int c) {
    KNN::k = k;
    KNN::c = c;
}

float KNN::cal_distance(std::vector<float> v_a, std::vector<float> v_b) {
    assert( v_a.size() == v_b.size());
    float dis = 0.0;
    int len = v_a.size();
    for( int i = 0; i < len; i++) {
        dis += (v_a[i] - v_b[i]) * (v_a[i] - v_b[i]);
    }
    return dis;
}

KNN::KD_Node* KNN::insert(KNN::KD_Node* r, std::vector<float> vec, int label, int idx) {
    if(!r) return KNN::create_kd_node(vec, label);
    idx = idx%KNN::m;
    if(vec[idx] < r->vec[idx]) r->left = insert(r->left, vec, label, idx+1);
    else r->right = insert(r->right, vec, label, idx+1);
    return r;
}

KNN::KD_Node *KNN::create_kd_node(std::vector<float> vec, int label) {
    KD_Node* res = new KD_Node();
    res->vec = vec;
    res->label = label;
    return res;
}

void KNN::search_knn(KNN::KD_Node *r, std::vector<float> vec, int idx) {
    if(!r) return;
    float now_dis = KNN::cal_distance(vec, r->vec);
    KNN::pq.push(KNN::PQ_Node(now_dis, r->label, r->vec));
    if(KNN::pq.size() > k)  KNN::pq.pop();
    idx = idx%KNN::m;
    bool flag = pq.top().dis > (vec[idx] - r->vec[idx])*(vec[idx] - r->vec[idx]);
    if(vec[idx] < r->vec[idx]) {
        search_knn(r->left, vec, idx+1);
        if(flag) search_knn(r->right, vec, idx+1);
    }
    else {
        search_knn(r->right, vec, idx+1);
        if(flag) search_knn(r->left, vec, idx+1);
    }
}
