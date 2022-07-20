---
title: 匈牙利算法
date: 2022-06-09
categories: [算法和数据结构]
tags: [算法,目标关联]     # TAG names should always be lowercase
---

匈牙利算法是由匈牙利数学家Edmonds于1965年提出，因而得名。本质是为了解决二分图匹配问题。大致理解如下:  
有n个男的，m个女的。男的只能跟女的配对，且只能一对一配对。同时，如果男女配对需要消耗彩礼cost，不同男女之间的彩礼是不一样的。问如何尽可能的配对，同时配对所消耗的彩礼最小。匈牙利本质就是解决这种问题。具体可以看[匈牙利算法详解](https://blog.csdn.net/sunny_hun/article/details/80627351),此算法常用于**目标关联，目标融合，目标匹配**等场景。本文主要记录如何实现。

### cost相同，尽可能匹配多

```cpp
#include <bits/stdc++.h>
#define INF 0x3f3f3f3f;
using namespace std;
int L[505][505];
int boy[505];
int used[505];
int k,m,n;  //可能的组合数k，女生数m，男生数n 

// 二分图匹配 
bool find(int i){
    for(int j=1;j<=n;j++){
        if(L[i][j] && !used[j]){  //跟他有关系而且没有搜索过 
            used[j]=1;
            if(!boy[j] || find(boy[j])){
                boy[j]=i;
                return true;
            }
        }
    }
    return false;
}


int main(){
    while(cin>>k>>m>>n){
        memset(L,0,sizeof(L));
        memset(boy,0,sizeof(boy));
        for(int i=1;i<=k;i++){
            int n1,n2;
            cin>>n1>>n2;
            L[n1][n2]=1;
        }
        int sum=0;
        for(int i=1;i<=m;i++){
            memset(used,0,sizeof(used));
            if(find(i)) sum++;
        }
        cout<<sum<<endl;
    }
    return 0;
} 

```

### cost不同，尽可能cost最小，递归方法
```cpp
#include <bits/stdc++.h>
using namespace std;
const int INF = 0x3f3f3f3f;

int love[305][305];    // 每个妹子对每个男生的好感度 
int ex_girl[305];      // 每个妹子的期望值
int ex_boy[305];       // 每个男生的期望值
bool vis_girl[305];    // 每一轮匹配匹配过的女生
bool vis_boy[305];     // 每一轮匹配匹配过的男生
int match[305];        // 每个男生匹配到的妹子 如果没有则为-1
int slack[305];        // 每个汉子如果能被妹子倾心最少还需要多少期望值
int n;

bool dfs(int girl){
    vis_girl[girl] = true;
    for (int boy = 0; boy < n; boy++) {
        if (vis_boy[boy]) continue; // 每一轮匹配 每个男生只尝试一次
        int gap = ex_girl[girl] + ex_boy[boy] - love[girl][boy];
        if (gap == 0) {  // 如果符合要求
            vis_boy[boy] = true;
            if (match[boy] == -1 || dfs( match[boy] )) {    // 找到一个没有匹配的男生 或者该男生的妹子可以找到其他人
                match[boy] = girl;
                return true;
            }
        }else{
            slack[boy] = min(slack[boy], gap);  // slack 可以理解为该男生要得到女生的倾心 还需多少期望值 取最小值 备胎的样子
        }
    }
    return false;
}
int KM(){
    memset(match, -1, sizeof match);    // 初始每个男生都没有匹配的女生
    memset(ex_boy, 0, sizeof ex_boy);   // 初始每个男生的期望值为0
    // 每个女生的初始期望值是与她相连的男生最大的好感度
    for (int i = 0; i < n; i++) {
        ex_girl[i] = love[i][0];
        for (int j = 1; j < n; j++) {
            ex_girl[i] = max(ex_girl[i], love[i][j]);
        }
    }
    // 尝试为每一个女生解决归宿问题
    for (int i = 0; i < n; i++) {
        fill(slack, slack + n, INF);    // 因为要取最小值 初始化为无穷大
        while(1){
            // 为每个女生解决归宿问题的方法是 ：如果找不到就降低期望值，直到找到为止
            // 记录每轮匹配中男生女生是否被尝试匹配过
            memset(vis_girl, false, sizeof vis_girl);
            memset(vis_boy, false, sizeof vis_boy);
            if(dfs(i)) break;  // 找到归宿 退出
            // 如果不能找到 就降低期望值
            // 最小可降低的期望值
            int d = INF;
            for (int j = 0; j < n; j++)
                if (!vis_boy[j])    d = min(d, slack[j]);
            for (int j = 0; j < n; j++) {
                // 所有访问过的女生降低期望值
                if (vis_girl[j]) ex_girl[j] -= d;
                // 所有访问过的男生增加期望值
                if (vis_boy[j]) ex_boy[j] += d;
                // 没有访问过的boy 因为girl们的期望值降低，距离得到女生倾心又进了一步！
                else slack[j] -= d;
            }
        }
    }
    // 匹配完成 求出所有配对的好感度的和
    int res = 0;
    for (int i = 0; i < n; i++)
        res += love[match[i]][i];
    return res;
}
int main(){
    while (cin>>n) {
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                cin>>love[i][j];
        cout<<KM()<<endl;
    }
    return 0;
}        

```

### cost不同，尽可能cost最小，矩阵方法

```python

import numpy as np

class Munkres:

    def __init__(self, cost: list, inv_eps=1000) -> None:
        """[summary]
        https://brc2.com/the-algorithm-workshop/
        Args:
            cost (list): [二维权值方阵]
        """

        self.cost = np.array(cost) * inv_eps
        self.cost.astype(np.int32)
        self.run_cost = self.cost
        self.rows = len(cost)
        self.cols = len(cost[0])
        self.step = 1
        self.running = True
        assert(self.rows == self.cols)

        self.mp = {
            1: self.step_one,
            2: self.step_two,
            3: self.step_three,
            4: self.step_four,
            5: self.step_five,
            6: self.step_six,
            7: self.step_seven
        }

        self.mask = np.zeros((self.rows, self.cols))
        self.row_cover = np.zeros(self.rows)
        self.col_cover = np.zeros(self.cols)

        self.paths = []

    def step_one(self):
        """[summary]
            For each row of the matrix, find the smallest element and subtract it from every element in its row.  Go to Step 2
        """
        for i in range(self.rows):
            self.run_cost[i] -= min(self.run_cost[i])
        self.step = 2

    def step_two(self):
        """[summary]
        Find a zero (Z) in the resulting matrix. If there is no starred zero in its row or column, star Z. Repeat for each element in the matrix. Go to Step 3
        """
        for i in range(self.rows):
            for j in range(self.cols):
                if self.run_cost[i][j] == 0 and self.row_cover[i] == 0 and self.col_cover[j] == 0:
                    self.mask[i][j] = 1
                    self.row_cover[i] = 1
                    self.col_cover[j] = 1

        for i in range(self.rows):
            self.row_cover[i] = 0
        for j in range(self.cols):
            self.col_cover[j] = 0

        self.step = 3

    def step_three(self):
        """[summary]
            Cover each column containing a starred zero.  
            If K columns are covered, the starred zeros describe a complete set of unique assignments.  
            In this case, Go to DONE, otherwise, Go to Step 4.
        """
        for i in range(self.rows):
            for j in range(self.cols):
                if self.mask[i][j] == 1:
                    self.col_cover[j] = 1

        colcount = np.sum(self.col_cover)
        if colcount >= self.rows or colcount >= self.cols:
            self.step = 7
        else:
            self.step = 4

    def __find_a_zero(self):
        """[summary]
         Find a noncovered zero
        Returns:
            [type]: [row, col , default -1]
        """
        r, c = -1, -1
        for i in range(self.rows):
            for j in range(self.cols):
                if self.run_cost[i][j] == 0 and self.row_cover[i] == 0 and self.col_cover[j] == 0:
                    return i, j
        return r, c

    def __find_star_in_row(self, row):
        """[summary]

        Args:
            row ([type]): [row]

        Returns:
            [int]: [find stared col in row, default -1]
        """
        for j in range(self.cols):
            if self.mask[row][j] == 1:
                return j
        return -1

    def step_four(self):
        """[summary]
            Find a noncovered zero and prime it.  If there is no starred zero in the row containing this primed zero, Go to Step 5.  
            Otherwise, cover this row and uncover the column containing the starred zero. 
            Continue in this manner until there are no uncovered zeros left. Save the smallest uncovered value and Go to Step 6.
        """
        done = False
        while not done:
            noncover_r, noncover_c = self.__find_a_zero()
            if noncover_r == -1:
                done = True
                self.step = 6
            else:
                self.mask[noncover_r][noncover_c] = 2
                star_col = self.__find_star_in_row(noncover_r)
                if star_col != -1:
                    self.row_cover[noncover_r] = 1
                    self.col_cover[star_col] = 0
                else:
                    done = True
                    self.step = 5
                    self.paths.append((noncover_r, noncover_c))

    def __find_star_in_col(self, col):
        for i in range(self.rows):
            if self.mask[i][col] == 1:
                return i
        return -1

    def __find_prime_in_row(self, row):
        """[summary]
        Args:
            col ([type]): [col]
        Returns:
            [int]: [find prime row in col, default -1]
        """
        for j in range(self.cols):
            if self.mask[row][j] == 2:
                return j
        return -1

    def step_five(self):
        """[summary]
            Construct a series of alternating primed and starred zeros as follows.  Let Z0 represent the uncovered primed zero found in Step 4. 
            Let Z1 denote the starred zero in the column of Z0 (if any). Let Z2 denote the primed zero in the row of Z1 (there will always be one).  
            Continue until the series terminates at a primed zero that has no starred zero in its column. 
            Unstar each starred zero of the series, star each primed zero of the series, erase all primes and uncover every line in the matrix.  Return to Step 3
        """
        
        done = False
        while not done:
            star_r = self.__find_star_in_col(self.paths[-1][1])
            if star_r > -1:
                self.paths.append( (star_r, self.paths[-1][1]) )
            else:
                done = True

            if not done:
                prime_c = self.__find_prime_in_row( self.paths[-1][0] )
                self.paths.append( (self.paths[-1][0], prime_c))

        # argument path
        for i, j in self.paths:
            if self.mask[i][j] == 1:
                self.mask[i][j] = 0
            else:
                self.mask[i][j] = 1

        # clear covers
        for i in range(self.rows):
            self.row_cover[i] = 0
        for j in range(self.cols):
            self.col_cover[j] = 0

        # erase prime
        for i in range(self.rows):
            for j in range(self.cols):
                if self.mask[i][j] == 2:
                    self.mask[i][j] = 0
        self.paths.clear()
        self.step = 3

    def step_six(self):
        """[summary]
            Add the value found in Step 4 to every element of each covered row, and subtract it from every element of each uncovered column.  
            Return to Step 4 without altering any stars, primes, or covered lines
        """
        minval = 1 << 31
        for i in range(self.rows):
            for j in range(self.cols):
                if self.row_cover[i] == 0 and self.col_cover[j] == 0:
                    minval = min(self.run_cost[i][j], minval)

        for i in range(self.rows):
            for j in range(self.cols):
                if self.row_cover[i] == 1:
                    self.run_cost[i][j] += minval
                if self.col_cover[j] == 0:
                    self.run_cost[i][j] -= minval

        self.step = 4


    def step_seven(self):
        # print("done !")
        # print(self.run_cost)
        # print(self.mask)
        self.running = False


    def run(self):
        while self.running:
            # print(self.step)
            self.mp[self.step]()
            # print(self.run_cost)
            # print("")

    def get_result(self):
        res = []
        vis = [0] * self.cols
        for i in range(self.rows):
            for j in range(self.cols):
                if self.mask[i][j] == 1 and vis[j] == 0:
                    res.append(j)
                    vis[j] = 1
                    break

        if len(res) != self.rows:
            print("algorithm error ...")
            return None
        return res

if  __name__ == "__main__":

    cost = [ [1.2, 1., 1.], [1., 1.2, 1.], [1., 1., 1.2]]
    mkr = Munkres(cost)
    mkr.run()
    print(mkr.get_result())

```

以上三种方式中，第三种广泛使用目标关联中。其具体原理见[Munkres’ Assignment Algorithm](https://brc2.com/the-algorithm-workshop/)