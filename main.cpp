#include <iostream>
#include "KMEANS.h"
#include <cstdlib>
#include "KNN.h"
using namespace std;


int main() {
    cout << "Hello, world!" << endl;

    KMEANS kmeans(10);

    vector< vector<float > > data(10000);

    for (int i = 0; i < 10000; ++i) {
        for (int j = 0; j < 20; ++j) {
            data[i].push_back(random()%5);
        }
    }
    kmeans.fit(data,10000);

    for (int k = 0; k < kmeans.k; ++k) {
        for (int i = 0; i < 20; ++i) {
            cout << kmeans.center[k][i] << " ";
        }
        cout << endl;
    }
    vector<int > res = kmeans.predict(data);
    for(int i = 0; i < 10; i++) cout << res[i] << " ";
    cout << endl;

    KNN knn = KNN(2, 2);

    vector< vector<float > > t_data(4);
    vector<int> labels;
    for (int l = 0; l < 2; ++l) {
        for (int i = 0; i < 2; ++i) {
            t_data[l*2+i].push_back(l);
            t_data[l*2+i].push_back(i);
        }
    }
    labels.push_back(0);
    labels.push_back(0);
    labels.push_back(1);
    labels.push_back(1);

    knn.fit(t_data, labels);
    t_data[3].clear();
    t_data[3].push_back(3);
    t_data[3].push_back(2);
    labels = knn.predict(t_data);
    for (int m = 0; m < 4 ; ++m) {
        cout << labels[m] << " ";
    }
    return 0;
}
