//
// Created by Kaihua Li on 2022/7/4.
//

#ifndef ESTAR_GO_GOBOARD_H
#define ESTAR_GO_GOBOARD_H


#include <cassert>
#include <string>
#include <map>
#include <vector>
#include <deque>
#include <cstdint>
#include <iostream>

#include "Utils.h"

extern "C" {
#include "../external/pachi/board.h"
#include "../external/pachi/board.c"
}


static const int N_FEATURE=5;
static const int N_PAT3=0xffffff;
static const int N_PAT12=0xffffff;
static const int N_NBR=8;

typedef struct {
    float pattern3w[N_PAT3];
    float pattern12w[N_PAT12];
    float neighborw[N_NBR];
    float alfinalw[N_FEATURE];
    float alfinalb[19][19];
}prior_t;
//                             S  N   E   W   NE SE  SW  NW SS  NN  EE WW
static const int __d12x[12] = {0,  0,  1, -1,  1, 1, -1, -1, 0,  0, 2, -2};
static const int __d12y[12] = {1, -1,  0,  0, -1, 1,  1, -1, 2, -2, 0, 0};

/**
 * The class representing a GO board
 * NOTE: This is a C++ Wrapper class of pachi's board representation in C.
 */
class GoBoard
{
private:
    struct board *b; /**< A pointer to pachi's GO board representation */
    std::string history;
    std::deque<uint64_t>  hash8;
    bool board_repetition;

/*    GoSet *dirty_pos[2];

    int p3hashesb[12][4][4];//direction*{NONE,MY,ENEMY,OFFBOARD}*{1,2,3+,notused}
    int p3hashesw[12][4][4];//direction*{NONE,MY,ENEMY,OFFBOARD}*{1,2,3+,notused}

    int dirhashes[12];//direction*/


    // each move 2 chars
    // color is implied in index. the first move is black.
    // normal move: col in A..S, row in a..s
    //     e.g. C4 => Cd
    // pass       : 'ZP'
    // resign     : 'ZR'
    void add_move_to_history(const int pos) {
        char hi, lo;
        if (POS_PASS==pos) {
            hi = 'Z', lo = 'P';
        } else if (POS_RESIGN==pos) {
            hi = 'Z', lo = 'R';
        } else {
            hi = (pos%19)+'A';
            lo = (pos/19)+'a';
        }
        history.append(1,hi);
        history.append(1,lo);
    }

    bool _apply_move(int pos,int color);
    void _recompute_libs(board* b, group_t g);
    bool _surround_by_atari_group(group_t g);
    void _update_feature(int my_color_int);

public:
    static const int COLOR_BLACK=0;
    static const int COLOR_WHITE=1;
    static const int COLOR_EMPTY=-1;
    static const int POS_PASS=-1;
    static const int POS_RESIGN=-2;

#if 0
    void create_hash(){
        int h = 0x35373c;
        for (int i = 0; i < 12; i++) {
            p3hashesb[i][S_NONE][0] = (h = h * 16803-7)&0xffffff;
            p3hashesb[i][S_OFFBOARD][0] = (h = h * 16803-7)&0xffffff;
            p3hashesw[i][S_NONE][0] = p3hashesb[i][S_NONE][0];
            p3hashesw[i][S_OFFBOARD][0] = p3hashesb[i][S_OFFBOARD][0];
            for (int j=1;j<4;j++){
                p3hashesb[i][S_NONE][j] = p3hashesb[i][S_NONE][0];
                p3hashesb[i][S_OFFBOARD][j] = p3hashesb[i][S_OFFBOARD][0];
                p3hashesw[i][S_NONE][j] = p3hashesw[i][S_NONE][0];
                p3hashesw[i][S_OFFBOARD][j] = p3hashesw[i][S_OFFBOARD][0];
            }
            for (int j=0;j<4;j++){
                p3hashesb[i][S_BLACK][j] = (h = h * 16803-7)&0xffffff;//mine, S_BLACK when my color is black
                p3hashesb[i][S_WHITE][j] = (h = h * 16803-7)&0xffffff;//enemy
                p3hashesw[i][S_WHITE][j] = p3hashesb[i][S_BLACK][j]; 
                p3hashesw[i][S_BLACK][j] = p3hashesb[i][S_WHITE][j];
            }
            dirhashes[i]=(h = h * 16803-7)&0xffffff;
        }
/*        for(int i=0;i<12;i++){
            for(int j=0;j<4;j++){
                for(int k=0;k<4;k++){
                    fprintf(stderr, "%d %d %d %x\n",i,j,k,p3hashesb[i][j][k]);
                    fprintf(stderr, "%d %d %d %x\n",i,j,k,p3hashesw[i][j][k]);
                }
            }
        }*/
    }
#endif

    GoBoard(float komi, int size){
        b = board_new(size, NULL);
        char rule[100]="chinese";
        board_set_rules(b, rule);
        board_resize(b,19);
        board_clear(b);
        b->komi=komi;
        history.clear();
        hash8.resize(8);
        board_repetition = false;
/*        dirty_pos[0]=new GoSet();
        dirty_pos[1]=new GoSet();

        for(int i=0;i<19*19;i++){
            dirty_pos[0]->add(i);
            dirty_pos[1]->add(i);
        }
        create_hash();*/
    }

    GoBoard(const GoBoard &other){
        b = (board *) malloc(sizeof(*b));
        board_copy(b, other.b);
        history = other.history;
        hash8 = other.hash8;
        board_repetition = other.board_repetition;
        /*dirty_pos[0]=new GoSet(*(other.dirty_pos[0]));
        dirty_pos[1]=new GoSet(*(other.dirty_pos[1]));

        create_hash();*/
    }

    ~GoBoard() {
        board_done(b);
/*        delete dirty_pos[0];
        delete dirty_pos[1];*/
    }

    uint64_t get_hash() {
        return b->hash;
    }

    bool check_repetition(uint64_t cur_hash){
        std::deque<uint64_t>::iterator it = hash8.begin();
        bool is_repetition = false;
        while(it != hash8.end()){
            if(*it == cur_hash){
                is_repetition = true;
            }
            it++;
        }
        return is_repetition;
    }

    void store_hash(uint64_t cur_hash){
        if(hash8.size() < 8) {
            hash8.push_back(cur_hash);
        } else{
            hash8.pop_front();
            hash8.push_back(cur_hash);
        }
    }

    bool get_board_repetition(){
        return board_repetition;
    }

    void print_hash(){
        std::deque<uint64_t>::iterator it = hash8.begin();
        while(it != hash8.end()){
            std::cout<<*it<<std::endl;
            it++;
        }
    }

    void reset(){
        board_clear(b);
        history.clear();
    }

    void print_board() {
        print_board(stderr);
        fprintf(stderr, "history:%s\n",get_history().c_str());
    }

    std::string print_board_buffer();

    void print_board(FILE * f){
        board_print(b,f);
    }

    void set_komi(float komi) {
        b->komi = komi;
    }

    /**
     * Returns the color of next player.
     */
    int get_color() {
        return (history.length()%4)>>1;
    }

    float get_score_black() {
        return -board_official_score(b, NULL);
    }

    bool is_pass_safe(){
        float score = board_official_score(b, NULL);
        if (get_color() == COLOR_BLACK)
            score = -score;
        return (score >= 0);
    }

    std::string get_history() const {
        return history;
    }

    int get_total_moves() {
        return history.length()/2;
    }

    board* get_pachi_board(){
        return b;
    }

    bool apply_move(int action);
    bool apply_history(const std::string& history);

    //policynet and valuenet
    void extract_feature_stone(float* feature,int stride,int offset,int my_color_int);
    void extract_feature_valid_move(float* feature,int stride,int offset,int my_color_int);
    void extract_feature_sensible(float* feature,int stride,int offset,int my_color_int);
    void extract_feature_liberty(float* feature,int stride,int offset,int max_liberty);
    void extract_feature_capture(float* feature,int stride,int offset,int my_color_int,int max_capture);
    void extract_feature_selfatari_size(float* feature, int stride, int offset, int my_color_int, int max_atari);
    void extract_feature_lib_aftermove(float* feature,int stride,int offset,int my_color_int, int max_liberty);
    void extract_feature_ladder_capture_and_cann_escape(float* feature,int stride,int offset,int my_color_int);
    void extract_feature_turnsince(float* feature,int stride,int offset,int max_history);
    void extract_feature_dragon_safety(float* feature,int stride,int offset);
    void extract_feature_dragon_lib(float* feature,int stride,int offset);
    void extract_feature_lastko(float* feature,int stride,int offset);
    void extract_pachi_capture(float *feature,int stride, int offset, int my_color_int);
    void extract_pachi_selfatari(float *feature,int stride, int offset, int my_color_int);
    void extract_pachi_2lib(float *feature,int stride, int offset, int my_color_int);
    void extract_feature_dragon_old(float* feature,int stride,int offset);

    void extract_feature_dragon_mgroup(float* feature,int stride, int offset, int max_history);

    void extract_feature_dragon_group(int* feature);
    void extract_feature_group(int* feature);
    void extract_feature_planes(float* feature,int stride,int t, int next_color);
    void extract_feature_next_color(float* feature,int stride,int offset);
    void extract_feature_all_planes(float* feature, int stride, int next_color, std::string new_history);

#if 0
    #ifdef CALCULATE
    //fastrollout
    float calculate_fastrollout_feature(int pos, prior_t &prior, int my_color_int);
    float calculate_response_feature(int pos, int dir, prior_t &prior, int my_color);
#endif

#ifdef EXTRACT
    void extract_fastrollout_feature(float* feature,int *pat3, int my_color_int);
    void extract_response_feature(int* patr12, int* nbr, int my_color_int);
#endif

    GoSet *get_dirty(int my_color_int){
        if(b->dirty_size>0){
            for(int i=0;i<b->dirty_size;i++){
                dirty_pos[0]->add(b->dirty[i]);
                dirty_pos[1]->add(b->dirty[i]);
            }
        }
        b->dirty_size=0;
        return dirty_pos[my_color_int];
    }
#endif
};


#endif //ESTAR_GO_GOBOARD_H
