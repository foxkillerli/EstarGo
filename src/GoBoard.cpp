//
// Created by Kaihua Li on 2022/7/4.
//

#include <algorithm>
#include <cmath>
#include <cstdint>
#include "GoBoard.h"

extern "C" {
#include "../external/pachi/board.h"
#include "../external/pachi/move.h"
#include "../external/pachi/tactics/ladder.h"
#include "../external/pachi/tactics/nakade.h"
#include "../external/pachi/tactics/selfatari.h"
#include "../external/pachi/tactics/dragon.h"
#include "../external/pachi/tactics/2lib.h"
#include "../external/pachi/tactics/1lib.h"
}

// Reset GoBoard to initial state
bool GoBoard::reset_board() {
    board_clear(b);
    history.clear();
    hash8.clear();
    board_repetition = false;
    return true;
}

bool GoBoard::apply_move(int action) {
    bool succ = true;
    if(action>=0){
        succ=_apply_move_game(action,get_color());
        if(succ){
            add_move_to_history(action);
        }
    }else{
        add_move_to_history(action);
    }
    return succ;
}
// apply move to board
// one step for black, then step for white in sequence
bool GoBoard::_apply_move_game(int action, int color) {
    move_t m;
    m.color = color == COLOR_BLACK ? S_BLACK : S_WHITE;
    m.coord = coord_xy(action % 19+1,action / 19+1);
    bool succ = true;
    if (board_is_valid_move(b, &m)){
        board_play(b, &m);
    }else{
        succ = false;
    }
    uint64_t cur_hash = get_hash();
    if(!check_repetition(cur_hash)){
        store_hash(cur_hash);
    }
    else {
        board_repetition = true;
    }
    return succ;
}

// apply move to board without check color
bool GoBoard::_apply_move_setting(int action, int color) {
    move_t m;
    m.color = color == COLOR_BLACK ? S_BLACK : S_WHITE;
    m.coord = coord_xy(action % 19+1,action / 19+1);
    bool succ = true;
    if (board_is_valid_move(b, &m)){
        board_play(b, &m);
    }else{
        succ = false;
    }
    uint64_t cur_hash = get_hash();
    if(!check_repetition(cur_hash)){
        store_hash(cur_hash);
    }
    else {
        board_repetition = true;
    }
    return succ;
}
/*
std::string GoBoard::print_board_buffer() {
    std::string buffer(10240, ' ');
    int board_size = board_print(b, const_cast<char *>(buffer.data()), buffer.size());
    buffer.resize(board_size);
    return buffer;
}
*/
bool GoBoard::apply_history(const std::string& history) {
    assert(history.length() % 2 == 0);
    this->history=history;
    for (std::string::size_type step = 0; step < history.length() / 2; ++step){
        char hi=history[2*step];
        char lo=history[2*step+1];

        if (hi!='Z') {
            int pos=(hi-'A')+(lo-'a')*19;
            bool succ=_apply_move_game(pos,step%2);

            if(!succ){
                print_board();
                fprintf(stderr, "%s %lu\n",history.c_str(),step);
            }
            /*if(!succ)
                return false;*/
            assert(succ);
        }
    }
    return true;
}




