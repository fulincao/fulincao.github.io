#ifndef CMUNKRES_H
#define CMUNKRES_H
#include <vector>
#include <stdint.h>

namespace FRAMEWORK {
    class CMunkres {
    public:
        CMunkres();

        virtual ~CMunkres();

        void init(int dim);

        float assignDetectionsToTracks(const std::vector<std::vector<float>> &cost, float cost_not_asso,
                                       std::vector<int32_t> &track_indices_asso,
                                       std::vector<int32_t> &mea_indices_asso,
                                       std::vector<int32_t> &track_indices_not_asso,
                                       std::vector<int32_t> &mea_indices_not_asso,
                                       float invalid_cost = -1, bool padding = true);

        void paddingMatrix(const std::vector<std::vector<float>> &cost, float cost_not_asso, std::vector<std::vector<float>> &padding_cost,
                           float invalid_cost = -1);

        void regulateCost(std::vector<std::vector<float>> &padding_cost);

        void starCost(const std::vector<std::vector<float>> &padding_cost);

        bool coverCol(const std::vector<std::vector<float>> &padding_cost);

        float getResult(const std::vector<std::vector<float>> &org_cost,
                        std::vector<int32_t> &track_indices_asso,
                        std::vector<int32_t> &mea_indices_asso,
                        std::vector<int32_t> &track_indices_not_asso,
                        std::vector<int32_t> &mea_indices_not_asso);

        bool coverRow(const std::vector<std::vector<float>> &padding_cost, float &min_value, std::vector<int> &last_prime_pos);

        void updateLableAndCovers(const std::vector<int> &start_prime_pos);

        void updateCost(std::vector<std::vector<float>> &padding_cost, float min_value);

    protected:

    private:
        std::vector<std::vector<int>> zero_pos_per_row;
        std::vector<std::vector<int>> zero_pos_per_col;
        std::vector<std::vector<int>> star_pos_per_row;
        std::vector<std::vector<int>> star_pos_per_col;
        std::vector<std::vector<int>> prime_pos_per_row;
        std::vector<std::vector<int>> prime_pos_per_col;

        std::vector<int> cover_col;
        std::vector<int> cover_row;

        std::vector<int> assi;

    };
}
#endif // CMUNKRES_H
