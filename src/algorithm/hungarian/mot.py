'''
Description: multi object tracking
Author: caofulin@minieye.cc
FilePath: /obj_evaluation/measure_judge/common/mot.py
Date: 2021-09-24 19:37:53
'''

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
