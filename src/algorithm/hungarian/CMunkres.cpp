#include "CMunkres.h"
#include <algorithm>
#include <limits.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

#include <float.h>
//#include "Cfg.h"
#include "Log.h"
#include <math.h>
using namespace std;
namespace FRAMEWORK {
#define FLT_EQUAL(f1, f2, eps) (fabs((f1)-(f2))<(eps))
#define FLT_IS_ZERO(f) FLT_EQUAL(f,0,0.00000001)
    template<typename T>
    static void util_print_matrix(const vector<vector<T>> &m) {
#ifdef DEBUG
        //if (FLAGS_LOG_level.find("vv") != FLAGS_LOG_level.npos) {
            for (size_t i = 0; i < m.size(); i++) {
                for (size_t j = 0; j < m[i].size(); j++) {
                    cout << m[i][j] << " ";
                }
                cout << endl;
            }
            cout << endl;
        //}
#endif

    }

    template<typename T>
    static void util_save_matlab_matrix(const vector<vector<T>> &m, string name) {
#ifdef DEBUG
        if (FLAGS_LOG_level.find("vv") != FLAGS_LOG_level.npos) {

            string fp = name;
            ofstream f(fp.c_str());
            for (size_t i = 0; i < m.size(); i++) {
                for (size_t j = 0; j < m[i].size(); j++) {
                    if (j == m[i].size() - 1) {
                        f << setw(10) << left << m[i][j];
                    } else {
                        f << setw(10) << left << m[i][j] << ",";
                    }
                }
                f << ";\n";
            }
            f.close();
        }
#endif
    }

    template<typename T>
    static void util_print_vector(const vector<T> &v) {
#ifdef DEBUG
        //if (FLAGS_LOG_level.find("vv") != FLAGS_LOG_level.npos) {

            for (auto i:v) {
                cout << i << " ";
            }
            cout << endl;
        //}
#endif
    }

    CMunkres::CMunkres() {
        //ctor
        zero_pos_per_row.reserve(128);
        zero_pos_per_col.reserve(128);
        star_pos_per_row.reserve(128);
        star_pos_per_col.reserve(128);
        prime_pos_per_row.reserve(128);
        prime_pos_per_col.reserve(128);
        cover_col.reserve(128);
        cover_row.reserve(128);
        assi.reserve(128);

    }

    CMunkres::~CMunkres() {
        //dtor
    }

    void CMunkres::init(int dim) {
        star_pos_per_col.clear();
        star_pos_per_row.clear();
        prime_pos_per_col.clear();
        prime_pos_per_row.clear();
        zero_pos_per_row.clear();
        zero_pos_per_col.clear();
        cover_col.clear();
        cover_row.clear();
        assi.clear();

        for (int i = 0; i < dim; i++) {
            zero_pos_per_row.push_back(vector<int>());
            zero_pos_per_col.push_back(vector<int>());
            star_pos_per_row.push_back(vector<int>());
            star_pos_per_col.push_back(vector<int>());
            prime_pos_per_row.push_back(vector<int>());
            prime_pos_per_col.push_back(vector<int>());
        }

        assi.insert(assi.end(), dim, -1);
    }

    //http://csclab.murraystate.edu/~bob.pilgrim/445/munkres.html
    float CMunkres::assignDetectionsToTracks(const vector<vector<float>> &cost, float cost_not_asso,
                                             vector<int32_t> &track_indices_asso,
                                             vector<int32_t> &mea_indices_asso,
                                             vector<int32_t> &track_indices_not_asso,
                                             vector<int32_t> &mea_indices_not_asso,
                                             float invalid_cost/*=-1*/, bool padding/*=true*/) {
        //LD()<<"begin";
        float c = -1;

        int counter = 0;

        vector<vector<float>> padding_cost;
        if (padding) {
            paddingMatrix(cost, cost_not_asso, padding_cost, invalid_cost);
        } else {
            padding_cost = cost;
        }
        init(padding_cost.size());

#ifdef DEBUG
        //LV()<<"padding matrix:";
        util_print_matrix(padding_cost);
#endif

        int step = 1;
        float min_value;//for step 6
        vector<int> latest_prime_pos_for_step5;

        bool done = false;
        while (!done) {
            counter++;
            if(counter>=10000){
                LE()<<"time out>10000,you should pay attention to this";
                break;
            }
//            LD()<<"counter="<<counter;
//            std::cout<<"counter="<<counter<<std::endl;
            switch (step) {
                case 1:
#ifdef DEBUG
                    //LV()<<"Step 1:  For each row of the matrix, find the smallest element and subtract it from every element in its row.  Go to Step 2.";
#endif
                    regulateCost(padding_cost);
                    step = 2;
                    {
#ifdef DEBUG

                        string name = "regulateCost";
                        stringstream ss;
                        ss << counter;
                        name += ss.str();
                        //LV()<<name<<":";
                        //vv("%s:", name.c_str());
                        util_print_matrix(padding_cost);
                        util_save_matlab_matrix(padding_cost, name);
#endif
                    }
                    break;
                case 2:

                    starCost(padding_cost);
                    step = 3;
                    break;
                case 3:

                    if (coverCol(padding_cost)) {
                        done = true;
                        c = getResult(cost, track_indices_asso, mea_indices_asso, track_indices_not_asso,
                                      mea_indices_not_asso);
                    } else {
                        step = 4;
                    }
                    break;
                case 4:

                    if (coverRow(padding_cost, min_value, latest_prime_pos_for_step5)) {
                        step = 6;
                    } else {
                        step = 5;
                    }
                    break;
                case 5:

                    updateLableAndCovers(latest_prime_pos_for_step5);
                    step = 3;
                    break;
                case 6:

                    updateCost(padding_cost, min_value);
                    step = 4;
                    {
#ifdef DEBUG

                        string name = "updateCost";
                        stringstream ss;
                        ss << counter;
                        name += ss.str();
                        //vv("%s:", name.c_str());
                        //LV()<<name<<":";
                        util_print_matrix(padding_cost);
                        util_save_matlab_matrix(padding_cost, name);
#endif
                    }
                    break;
            }
        }
        //LD()<<"end";
        return c;

    }

    void
    CMunkres::paddingMatrix(const vector<vector<float>> &cost, float cost_not_asso, vector<vector<float>> &padding_cost,
                            float invalid_cost/*=-1*/) {
        int row = cost.size();
        int col = cost[0].size();
        for (int i = 0; i < row + col; i++) {
            vector<float> one_row;
            padding_cost.push_back(one_row);
            if (i < row) {
                padding_cost[i].insert(padding_cost[i].end(), cost[i].begin(), cost[i].end());
                padding_cost[i].insert(padding_cost[i].end(), row, FLT_EQUAL(invalid_cost,-1,0.00001) ? FLT_MAX : invalid_cost);
                padding_cost[i][i + col] = cost_not_asso;
            } else {
                padding_cost[i].insert(padding_cost[i].end(), col, FLT_EQUAL(invalid_cost,-1,0.00001) ? FLT_MAX : invalid_cost);
                padding_cost[i].insert(padding_cost[i].end(), row, 0);
                padding_cost[i][i - row] = cost_not_asso;
            }
        }
    }

    void CMunkres::regulateCost(vector<vector<float>> &padding_cost) {
        //Step 1:  For each row of the matrix, find the smallest element and subtract it from every element in its row.
        int row = padding_cost.size();
        int col = padding_cost[0].size();
        for (int i = 0; i < row; i++) {
            auto min_it = min_element(padding_cost[i].begin(), padding_cost[i].end());
            auto min_value = *min_it;
            for (int j = 0; j < col; j++) {
                padding_cost[i][j] -= min_value;
                if (FLT_IS_ZERO(padding_cost[i][j])) {
                    zero_pos_per_row[i].push_back(j);
                    zero_pos_per_col[j].push_back(i);
                }
            }
        }

    }

    void CMunkres::starCost(const vector<vector<float>> &padding_cost) {
        //Step 2:  Find a zero (Z) in the resulting matrix.  If there is no starred zero in its row or column, star Z.
        // Repeat for each element in the matrix. Go to Step 3.

        int no_zero_pos_per_row = static_cast<int>(zero_pos_per_row.size());
        for (int row = 0; row < no_zero_pos_per_row; row++) {
            auto col_it = zero_pos_per_row[row].begin();
            while (col_it != zero_pos_per_row[row].end()) {
                if (star_pos_per_row[row].size() == 0 && star_pos_per_col[*col_it].size() == 0) {
                    star_pos_per_row[row].push_back(*col_it);
                    star_pos_per_col[*col_it].push_back(row);

                    auto col_index = *col_it;
                    col_it = zero_pos_per_row[row].erase(col_it);
                    auto found_it = find(zero_pos_per_col[col_index].begin(), zero_pos_per_col[col_index].end(), row);
                    if (found_it == zero_pos_per_col[col_index].end()) {
                        //LE()<<"must has zero in zero_pos_per_col:"<<row<<","<<col_index;
                    } else {
                        zero_pos_per_col[col_index].erase(found_it);
                    }
                } else {
                    col_it++;
                }
            }
        }
#ifdef DEBUG

        //LV()<<"step 2,star_pos_per_row=";
        util_print_matrix(star_pos_per_row);
        //LV()<<"step 2,zero_pos_per_col=";
        util_print_matrix(zero_pos_per_col);
        //LV()<<"step 2,zero_pos_per_row=";
        util_print_matrix(zero_pos_per_row);
#endif
    }

    bool CMunkres::coverCol(const vector<vector<float>> &padding_cost) {
        //Step 3:  Cover each column containing a starred zero.  If K columns are covered, the starred zeros describe a complete set of unique assignments.
        // In this case, Go to DONE, otherwise, Go to Step 4.
        assi.clear();
        assi.insert(assi.end(), padding_cost.size(), -1);
        int no_star_pos_per_col = static_cast<int>(star_pos_per_col.size());
        for (int col = 0; col < no_star_pos_per_col; col++) {
            if (star_pos_per_col[col].size() > 0) {
                cover_col.push_back(col);
                if (star_pos_per_col[col].size() != 1) {
                    //LW()<<"must has only one starred zero in one col:"<<col<<","<<star_pos_per_col[col].size();
                }
                assi[star_pos_per_col[col][0]] = col;
            }
        }
#ifdef DEBUG

        //LV()<<"step 3,cover_col=";
        util_print_vector(cover_col);
#endif
        return cover_col.size() == star_pos_per_col.size();
    }

    float CMunkres::getResult(const vector<vector<float>> &org_cost,
                              vector<int32_t> &track_indices_asso,
                              vector<int32_t> &mea_indices_asso,
                              vector<int32_t> &track_indices_not_asso,
                              vector<int32_t> &mea_indices_not_asso) {
        float c = 0;
        int org_cost_row = org_cost.size();
        int org_cost_col = org_cost[0].size();
        int no_assi = static_cast<int>(assi.size());
        for (int i = 0; i < no_assi; i++) {
            if (i < org_cost_row) {
                if (assi[i] < org_cost_col) {
                    track_indices_asso.push_back(i);
                    mea_indices_asso.push_back(assi[i]);
                    c += org_cost[i][assi[i]];
                } else {
                    track_indices_not_asso.push_back(i);
                }
            } else {
                if (assi[i] < org_cost_col) {
                    mea_indices_not_asso.push_back(i - org_cost_row);
                }
            }
            //track_indices_not_asso
            //mea_indices_not_asso
        }
        return c;
    }

    bool CMunkres::coverRow(const vector<vector<float>> &padding_cost, float &min_value, vector<int> &last_prime_pos) {
        //Step 4:  Find a noncovered zero and prime it.  If there is no starred zero in the row containing this primed zero, Go to Step 5.
        //  Otherwise, cover this row and uncover the column containing the starred zero.
        //Continue in this manner until there are no uncovered zeros left. Save the smallest uncovered value and Go to Step 6.
        int no_zero_pos_per_row = static_cast<int>(zero_pos_per_row.size());
        for (int row_index = 0; row_index < no_zero_pos_per_row; row_index++) {
            auto col_index_it = zero_pos_per_row[row_index].begin();
            while (col_index_it != zero_pos_per_row[row_index].end()) {
                if (find(cover_col.begin(), cover_col.end(), *col_index_it) == cover_col.end() &&
                    find(cover_row.begin(), cover_row.end(), row_index) == cover_row.end()) {
                    //Find a noncovered zero and prime it
                    prime_pos_per_row[row_index].push_back(*col_index_it);
                    prime_pos_per_col[*col_index_it].push_back(row_index);
                    //remove zero
                    auto col_index = *col_index_it;
                    col_index_it = zero_pos_per_row[row_index].erase(col_index_it);
                    auto found_it = find(zero_pos_per_col[col_index].begin(), zero_pos_per_col[col_index].end(),
                                         row_index);
                    if (found_it == zero_pos_per_col[col_index].end()) {
                        //LE()<<"must has zero in zero_pos_per_col:"<<row_index<<","<<col_index;
                    } else {
                        zero_pos_per_col[col_index].erase(found_it);
                    }

                    if (star_pos_per_row[row_index].size() == 0) {
                        //there is no starred zero in the row containing this primed zero, Go to Step 5
#ifdef DEBUG

                        //LV()<<"step 4.1,cover_row=";
                        util_print_vector(cover_row);
                        //LV()<<"step 4.1,cover_col=";
                        util_print_vector(cover_col);
                        //LV()<<"step 4.1,prime_pos_per_row=";
                        util_print_matrix(prime_pos_per_row);
                        //LV()<<"step 4.1,zero_pos_per_row=";
                        util_print_matrix(zero_pos_per_row);
                        //LV()<<"step 4.1,zero_pos_per_col=";
                        util_print_matrix(zero_pos_per_col);
#endif
                        last_prime_pos = {row_index, col_index};
                        return false;
                    } else {
                        //cover this row and uncover the column containing the starred zero.
                        cover_row.push_back(row_index);
                        if (star_pos_per_row[row_index].size() > 1) {
                            //LW()<<"more star in row:should uncover all the columns?"<<row_index<<","<<
                              star_pos_per_row[row_index].size();
                        }
                        auto found_col_it = find(cover_col.begin(), cover_col.end(), star_pos_per_row[row_index][0]);
                        if (found_col_it == cover_col.end()) {
                            //LE()<<star_pos_per_row[row_index][0]<<"col must be covered:"<<row_index;
                        }
                        cover_col.erase(found_col_it);
                    }
                } else {
                    col_index_it++;
                }
            }
        }

        min_value = FLT_MAX;
        for (size_t i = 0; i < padding_cost.size(); i++) {
            auto row_it = find(cover_row.begin(), cover_row.end(), i);
            if (row_it != cover_row.end()) {
                continue;
            }
            for (size_t j = 0; j < padding_cost[i].size(); j++) {
                auto col_it = find(cover_col.begin(), cover_col.end(), j);
                if (col_it == cover_col.end()) {
                    if (min_value > padding_cost[i][j]) {
                        min_value = padding_cost[i][j];
                    }
                }
            }
        }
#ifdef DEBUG

        //LV()<<"step 4.2,cover_row=";
        util_print_vector(cover_row);
        //LV()<<"step 4.2,cover_col=";
        util_print_vector(cover_col);
        //LV()<<"step 4.2,prime_pos_per_row=";
        util_print_matrix(prime_pos_per_row);
        //LV()<<"step 4.2,zero_pos_per_row=";
        util_print_matrix(zero_pos_per_row);
        //LV()<<"step 4.2,zero_pos_per_col=";
        util_print_matrix(zero_pos_per_col);
#endif
        //LV()<<"step 4.2,min_value="<<min_value;
        return true;

    }

    void CMunkres::updateLableAndCovers(const vector<int> &start_prime_pos) {
        //Step 5:  Construct a series of alternating primed and starred zeros as follows.
        //Let Z0 represent the uncovered primed zero found in Step 4.  Let Z1 denote the starred zero in the column of Z0 (if any).
        //Let Z2 denote the primed zero in the row of Z1 (there will always be one).
        // Continue until the series terminates at a primed zero that has no starred zero in its column.
        // Unstar each starred zero of the series, star each primed zero of the series, erase all primes and uncover every line in the matrix.
        // Return to Step 3.
        int prime_col_index = start_prime_pos[1];
        int prime_row_index = start_prime_pos[0];

        vector<vector<int>> prime_pos;
        vector<vector<int>> star_pos;
        if (prime_col_index == -1) {
            //LW()<<"is something wrong?";
        } else {
            //z0
            prime_pos.push_back(start_prime_pos);
        }

        while (prime_col_index != -1 && star_pos_per_col[prime_col_index].size() != 0) {
            if (star_pos_per_col[prime_col_index].size() > 1) {
                //LW()<<"only one star zero in prime's col?:"<<prime_col_index<<","<<
                  star_pos_per_col[prime_col_index].size();
            }

            //z1
            int starred_col_index = prime_col_index;
            int starred_row_index = star_pos_per_col[prime_col_index][0];
            star_pos.push_back({starred_row_index, starred_col_index});

            if (prime_pos_per_row[starred_row_index].size() == 0) {
                //LE()<<"there must always be one prime zero in row:"<<starred_row_index;
                break;
            } else {
                prime_row_index = starred_row_index;
                prime_col_index = prime_pos_per_row[starred_row_index][0];
                prime_pos.push_back({prime_row_index, prime_col_index});
            }
        }

        //Unstar each starred zero of the series, star each primed zero of the series, erase all primes and uncover every line in the matrix
        for (auto pos:star_pos) {
            star_pos_per_row[pos[0]].erase(
                    find(star_pos_per_row[pos[0]].begin(), star_pos_per_row[pos[0]].end(), pos[1]));
            star_pos_per_col[pos[1]].erase(
                    find(star_pos_per_col[pos[1]].begin(), star_pos_per_col[pos[1]].end(), pos[0]));
            zero_pos_per_row[pos[0]].push_back(pos[1]);
            zero_pos_per_col[pos[1]].push_back(pos[0]);
        }
        for (auto pos:prime_pos) {
            star_pos_per_row[pos[0]].push_back(pos[1]);
            star_pos_per_col[pos[1]].push_back(pos[0]);
        }
        for (size_t i = 0; i < prime_pos_per_row.size(); i++) {
            prime_pos_per_row[i].clear();
            //row==col
            prime_pos_per_col[i].clear();
        }
        cover_row.clear();
        cover_col.clear();
#ifdef DEBUG

        //LV()<<"step 5,prime_pos=";
        util_print_matrix(prime_pos);
        //LV()<<"step 5,star_pos=";
        util_print_matrix(star_pos);
        //LV()<<"step 5,star_pos_per_row=";
        util_print_matrix(star_pos_per_row);
        //LV()<<"step 5,zero_pos_per_row=";
        util_print_matrix(zero_pos_per_row);
        //LV()<<"step 5,zero_pos_per_col=";
        util_print_matrix(zero_pos_per_col);
#endif

    }

    void CMunkres::updateCost(vector<vector<float>> &padding_cost, float min_value) {
        //Step 6:  Add the value found in Step 4 to every element of each covered row, and subtract it from every element of each uncovered column.
        // Return to Step 4 without altering any stars, primes, or covered lines.

        vector<vector<int>> zero_pos_already;
        for (auto i:cover_row) {
            int no_padding_cost_i = static_cast<int>(padding_cost[i].size());
            for (int j = 0; j < no_padding_cost_i; j++) {
                if (FLT_IS_ZERO(padding_cost[i][j])) {
                    zero_pos_already.push_back({i, j});
                }
                padding_cost[i][j] += min_value;
            }
        }
        int no_padding_cost = static_cast<int>(padding_cost.size());
        for (int i = 0; i < no_padding_cost; i++) {
            int no_padding_cost_i = static_cast<int>(padding_cost[i].size());
            for (int j = 0; j < no_padding_cost_i; j++) {
                if (find(cover_col.begin(), cover_col.end(), j) == cover_col.end()) {
                    padding_cost[i][j] -= min_value;
                    if (FLT_IS_ZERO(padding_cost[i][j])) {
                        vector<int> pos = {i, j};
                        if (find(zero_pos_already.begin(), zero_pos_already.end(), pos) == zero_pos_already.end()) {
                            zero_pos_per_row[i].push_back(j);
                            zero_pos_per_col[j].push_back(i);
                        }
                    }
                }
            }
        }
#ifdef DEBUG

        //LV()<<"step 6,zero_pos_per_row=";
        util_print_matrix(zero_pos_per_row);
        //LV()<<"step 6,zero_pos_per_col=";
        util_print_matrix(zero_pos_per_col);
#endif
    }

}