//
// Created by cao on 2020/3/12.
//

#ifndef TEST_KMEANS_H
#define TEST_KMEANS_H

#include <vector>

class KMEANS {
public:
    int k;
    std::vector< std::vector<float > > center;
    KMEANS(int k=1);
    void fit(std::vector< std::vector<float> > data, int n_iter=10000);
    std::vector<int > predict(std::vector< std::vector<float> > data);

private:
    float cal_distance( std::vector<float > v_a, std::vector<float> v_b);

};

#endif //TEST_KMEANS_H
