//
// Created by cao on 2020/3/12.
//

#include "KMEANS.h"
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>

KMEANS::KMEANS(int k){
    assert( k > 0);
    KMEANS::k = k;
    KMEANS::center.resize(k);
}


void KMEANS::fit(std::vector< std::vector<float > >data, int n_iter) {
    if(data.empty()){
        printf("data is empty\n");
        return;
    }
    int len = data[0].size();
    int sz = data.size();
    bool mp[sz];
    for(int i = 0; i < KMEANS::k; i++ ){
        KMEANS::center[i].resize(len);

        while(true) {
            int rd = (int)random() % sz;
            if(!mp[rd]) {
                mp[rd] = true;
                for(int j = 0; j < len; j++) {
                    KMEANS::center[i][j] = data[rd][j];
                }
                break;
            }
        }
    }

    int i = 0;
    int last_labels[sz], now_labels[sz];
    std::vector< std::vector<float> > buf[KMEANS::k];
    while(i < n_iter){

        for(int j = 0; j < sz; j++){
            int label = 0;
            float dis = cal_distance(data[j], KMEANS::center[0]);

            for(int m = 1; m < KMEANS::k; m++){
                float tmp = cal_distance(data[j], KMEANS::center[m]);
                if(tmp < dis){
                    dis = tmp;
                    label = m;
                }
            }
            buf[label].push_back(data[j]);
            now_labels[j] = label;
        }

        for (int l = 0; l < KMEANS::k; ++l) {
            float one[len];
            for (int m = 0; m < len; ++m) {
                one[m] = 0;
            }

            int buf_sz = buf[l].size();
            for (int j = 0; j < buf_sz; ++j) {
                for (int n = 0; n < len; ++n) {
                    one[n] += buf[l][j][n];
                }
            }
            for (int i1 = 0; i1 < len; ++i1) {
                one[i1] = one[i1] / buf_sz;
                KMEANS::center[l][i1] = one[i1];
            }
        }

        for (int k1 = 0; k1 < KMEANS::k; ++k1) {
            buf[k1].clear();
        }

        bool change = false;
        float sse[KMEANS::k];
        memset(sse, 0, sizeof(sse));
        for (int j1 = 0; j1 < sz; ++j1) {
            if(last_labels[j1] != now_labels[j1]) {
                change = true;
            }
            // printf("j: %d, last_label: %d, now_label: %d\n", j1, last_labels[j1], now_labels[j1]);
            last_labels[j1] = now_labels[j1];
            sse[now_labels[j1]] += cal_distance(data[j1], KMEANS::center[now_labels[j1]] );
        }
        if(!change) break;
        i++;
        float tot_res = 0;
        for(int j2=0; j2<KMEANS::k; j2++) tot_res += sse[j2];
        printf("iter: %d, total sse: %.6lf\n", i, tot_res);
    }
}

std::vector<int> KMEANS::predict(std::vector< std::vector<float> > data){
    std::vector<int> res;
    int len = data.size();
    for(int i =0; i < len; i++){
        int label = 0;
        float dis = KMEANS::cal_distance(data[i], KMEANS::center[0]);
        for(int j = 1; j < KMEANS::k; j++){
            float tmp = KMEANS::cal_distance(data[i], KMEANS::center[j]);
            if(tmp < dis){
                dis = tmp;
                label = j;
            }
        }
        res.push_back(label);
    }
    return res;
}

float KMEANS::cal_distance(std::vector<float> v_a, std::vector<float> v_b) {
    assert( v_a.size() == v_b.size());
    float dis = 0.0;
    int len = v_a.size();
    for( int i = 0; i < len; i++) {
        dis += (v_a[i] - v_b[i]) * (v_a[i] - v_b[i]);
    }
    return dis;
}
