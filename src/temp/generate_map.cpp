//
// Created by cao on 2020/5/28.
//  n*n取m个联通点
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <queue>


using namespace std;

struct node {
    int x, y;
    bool operator < (const node a ) const {
        if(x != a.x )
            return x < a.x;
        return y < a.y;
    }
    node(int a, int b):x(a), y(b) {}
};


const int max_m = 50;
const int dir[4][2] = { {0, 1}, {0, -1}, {1, 0}, {-1, 0}};
int map[max_m][max_m];
int buf[max_m*max_m*max_m*max_m];
int res[max_m*max_m*max_m*max_m];
int n, m, flag, dep = 0;

void search( int x, int y) {
    buf[dep] = x*n + y;
    map[x][y] = 1;
    dep += 1;
//    printf("%d %d %d\n", x, y, dep);
    if(dep >= m ) {
//        sort(res, res+dep);
//        for (int i = 0; i < ; ++i) {
//
//        }
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (map[i][j]) printf("#");
                else printf(".");
            }
            printf("\n");
        }
        dep -= 1;
        map[x][y] = 0;
        return ;
    }

    for (int k = 0; k < 4; ++k) {
        int nx = x+dir[k][0];
        int ny = y+dir[k][1];
        if( nx >= n || nx < 0 || ny >= n || ny < 0 || map[nx][ny]) continue;
        search(nx, ny);
        dep -= 1;
        map[x][y] = 0;

    }
}


void bfs( int x, int y ) {

    queue<node> q;
    q.push(node(x, y));

    for( int i = 1 ; i < m) {

    }
}


int main(int argc, char* argv[]) {

    // scanf("input: %d %d %d", &n , &m, &flag);
    n = 5, m = 5;

    for( int i = 0; i < 1; i++)
        for( int j = 0; j < 1; j++) {
            memset(map, 0, sizeof(map));
            dep = 0;
            search(i, j);
            break;
        }

    return 0;
}

