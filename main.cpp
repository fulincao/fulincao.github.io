#include <iostream>
#include "KMEANS.h"
#include <cstdlib>
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

    for(int i = 0; i < 10; i++){
        cout << res[i] << endl;
    }

    return 0;
}
