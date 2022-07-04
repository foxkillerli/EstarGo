//
// Created by Kaihua Li on 2022/7/4.
//

#ifndef ESTAR_GO_SYMMETRY_H
#define ESTAR_GO_SYMMETRY_H
#include "utils/Logger.h"

inline std::pair<int, int> clk0(int row, int col) {
    return std::make_pair(row, col);
}
inline std::pair<int, int> revert_clk0(int row, int col) {
    return std::make_pair(row, col);
}

inline std::pair<int, int> clk90(int row, int col) {
    return std::make_pair(col, 18-row);
}
inline std::pair<int, int> revert_clk90(int row, int col) {
    return std::make_pair(18-col, row);
}

inline std::pair<int, int> clk180(int row, int col) {
    return std::make_pair(18-row, 18-col);
}
inline std::pair<int, int> revert_clk180(int row, int col) {
    return std::make_pair(18-row, 18-col);
}

inline std::pair<int, int> clk270(int row, int col) {
    return std::make_pair(18-col, row);
}
inline std::pair<int, int> revert_clk270(int row, int col) {
    return std::make_pair(col, 18-row);
}

inline std::pair<int, int> mclk0(int row, int col) {
    return std::make_pair(row, 18-col);
}
inline std::pair<int, int> revert_mclk0(int row, int col) {
    return std::make_pair(row, 18-col);
}

inline std::pair<int, int> mclk90(int row, int col) {
    return std::make_pair(18-col, 18-row);
}
inline std::pair<int, int> revert_mclk90(int row, int col) {
    return std::make_pair(18-col, 18-row);
}

inline std::pair<int, int> mclk180(int row, int col) {
    return std::make_pair(18-row, col);
}
inline std::pair<int, int> revert_mclk180(int row, int col) {
    return std::make_pair(18-row, col);
}

inline std::pair<int, int> mclk270(int row, int col) {
    return std::make_pair(col, row);
}
inline std::pair<int, int> revert_mclk270(int row, int col) {
    return std::make_pair(col, row);
}

typedef std::function<std::pair<int, int>(int row, int col)> Symfunc;

class Symmetry {
private:
    std::vector<Symfunc> symfunc_list;
    std::vector<Symfunc> revert_symfunc_list;
public:
    Symmetry() {
        symfunc_list.push_back(clk0);
        symfunc_list.push_back(clk90);
        symfunc_list.push_back(clk180);
        symfunc_list.push_back(clk270);
        symfunc_list.push_back(mclk0);
        symfunc_list.push_back(mclk90);
        symfunc_list.push_back(mclk180);
        symfunc_list.push_back(mclk270);

        revert_symfunc_list.push_back(revert_clk0);
        revert_symfunc_list.push_back(revert_clk90);
        revert_symfunc_list.push_back(revert_clk180);
        revert_symfunc_list.push_back(revert_clk270);
        revert_symfunc_list.push_back(revert_mclk0);
        revert_symfunc_list.push_back(revert_mclk90);
        revert_symfunc_list.push_back(revert_mclk180);
        revert_symfunc_list.push_back(revert_mclk270);
    }

    Symfunc get_symfunc(int symid) {
        if (symid >= (int)symfunc_list.size() || symid < 0) {
            LOG_FATAL("invalid symid:" << symid);
            symid = 0;
        }
        return symfunc_list[symid];
    }

    Symfunc get_revert_symfunc(int symid) {
        if (symid >= (int)revert_symfunc_list.size() || symid < 0) {
            LOG_FATAL("invalid symid:" << symid);
            symid = 0;
        }
        return revert_symfunc_list[symid];
    }

    std::string symmetry_history(std::string history, int symid) {
        Symfunc rotate = get_symfunc(symid);
        std::string new_history = "";
        for (int i = 0; i < (int)history.size()/2; ++i) {
            char hi = history[i*2];
            char lo = history[i*2+1];
            if (hi == 'Z' and lo == 'P') {
                new_history += "ZP";
                continue;
            }
            int col = hi - 'A';
            int row = lo - 'a';

            std::pair<int, int> new_row_col = rotate(row, col);
            int new_row = new_row_col.first;
            int new_col = new_row_col.second;
            char new_hi = new_col + 'A';
            char new_lo = new_row + 'a';
            new_history += new_hi;
            new_history += new_lo;
        }

        return new_history;
    }
};
#endif //ESTAR_GO_SYMMETRY_H
