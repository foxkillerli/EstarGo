//
// Created by Kaihua Li on 2022/7/4.
//

#include "../include/GoBoard.h"
#include <algorithm>
#include <cmath>
#include "GoBoard.h"
#include "utils.h"
#include "utils/Logger.h"

extern "C" {
#include "board.h"
#include "tactics/ladder.h"
#include "tactics/nakade.h"
#include "tactics/selfatari.h"
#include "tactics/dragon.h"
#include "tactics/2lib.h"
#include "tactics/1lib.h"
}

bool GoBoard::apply_move(int action) {
    bool succ = true;
    if(action>=0){
        succ=_apply_move(action,get_color());
        if(succ){
            add_move_to_history(action);
        }
    }else{
        add_move_to_history(action);
    }
    return succ;
}

bool GoBoard::_apply_move(int action, int color) {
    struct move m;
    m.color = color==COLOR_BLACK ? S_BLACK : S_WHITE;
    m.coord = coord_xy(b,action%19+1,action/19+1);
    bool succ=true;
    if (board_is_valid_move(b, &m)){
        board_play(b, &m);
    }else{
        succ=false;
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

std::string GoBoard::print_board_buffer() {
    std::string buffer(10240, ' ');
    int board_size = board_print_buffer(b, const_cast<char *>(buffer.data()), buffer.size());
    buffer.resize(board_size);
    return buffer;
}

bool GoBoard::apply_history(const std::string& history) {
    assert(history.length() % 2 == 0);
    this->history=history;
    for (std::string::size_type step = 0; step < history.length() / 2; ++step){
        char hi=history[2*step];
        char lo=history[2*step+1];

        if (hi!='Z') {
            int pos=(hi-'A')+(lo-'a')*19;
            bool succ=_apply_move(pos,step%2);

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

void GoBoard::extract_feature_sensible(float* feature,int stride,int offset,int my_color_int){
    enum stone my_color=(my_color_int==COLOR_BLACK?S_BLACK:S_WHITE);
    foreach_free_point(b){
        int x=coord_x(c,b);
        int y=coord_y(c,b);
        if(board_is_valid_play_no_suicide(b, my_color, c)){
            feature[(x+y*19-20)*stride+offset]=1.;
            if (!board_is_eyelike(b, c, my_color) || board_is_false_eyelike(b, c, my_color)){
                feature[(x+y*19-20)*stride+offset+1]=1.;
            }
        }
    }foreach_free_point_end;
}

//four layer: is_our,is_enemy,is_empty,const1
void GoBoard::extract_feature_stone(float* feature,int stride,int offset,int my_color_int){
    enum stone my_color=(my_color_int==COLOR_BLACK?S_BLACK:S_WHITE);
    int base=offset;
    foreach_point(b){
        enum stone color = board_at(b, c);
        if(color==S_NONE){
            feature[base+2]=1.;
            feature[base+3]=1.;
            base+=stride;
        }else if (color==my_color){
            feature[base]=1.;
            feature[base+3]=1.;
            base+=stride;
        }else if (color==S_OFFBOARD){
        }else{
            feature[base+1]=1.;
            feature[base+3]=1.;
            base+=stride;
        }
    }foreach_point_end;
}

void GoBoard::extract_feature_valid_move(float* feature,int stride,int offset,int my_color_int){
    enum stone my_color=(my_color_int==COLOR_BLACK?S_BLACK:S_WHITE);
    foreach_free_point(b){
        int x=coord_x(c,b);
        int y=coord_y(c,b);
        if(board_is_valid_play_no_suicide(b, my_color, c)){
            if (!board_is_eyelike(b, c, my_color) || board_is_false_eyelike(b, c, my_color)){

                feature[(x+y*19-20)*stride+offset]=1.;
            }
        }
    }foreach_free_point_end;
}

#define watermark_get(c)    (watermark[c >> 3] & (1 << (c & 7)))
#define watermark_set(c)    watermark[c >> 3] |= (1 << (c & 7))

void GoBoard::_recompute_libs(board* b, group_t g){
    struct group *gi=&board_group_info(b,g);
    unsigned char watermark[board_size2(b) / 8];
    memset(watermark, 0, sizeof(watermark));
    for (int i = 0; i < gi->libs; i++) {
        watermark_set(gi->lib[i]);
    }
    foreach_in_group(b, g) {
        coord_t coord2 = c;
        foreach_neighbor(b, coord2, {
                if (board_at(b, c) + watermark_get(c) != S_NONE)
                continue;
                watermark_set(c);
                if(gi->libs<GROUP_KEEP_LIBS-1){
                    gi->lib[gi->libs++] = c;
                }else{
                    return;
                }
        } );
    } foreach_in_group_end;
}

void GoBoard::extract_feature_liberty(float* feature,int stride,int offset,int max_liberty){
    for (group_t g = 1; g < board_size2(b); g++){
        if (group_at(b , g) == g){
            _recompute_libs(b,g);
            foreach_in_group(b, g) {
                int x=coord_x(c,b);
                int y=coord_y(c,b);
                feature[(x+y*19-20)*stride+offset+std::min(board_group_info(b, g).libs,max_liberty)-1]=1.;
            } foreach_in_group_end;
        }
    }
}

void GoBoard::extract_feature_capture(float* feature,int stride,int offset,int my_color_int, int max_capture){
    enum stone my_color=(my_color_int==COLOR_BLACK?S_BLACK:S_WHITE);
    foreach_free_point(b){
        if(board_is_valid_play_no_suicide(b, my_color, c)){
            int x=coord_x(c,b);
            int y=coord_y(c,b);
            if(trait_at(b,c,my_color).cap>0){
                int n_capture=0;
                with_move(b,c,my_color,{
                        n_capture=u_.captures;
                });
                assert(n_capture!=0);
                feature[(x+y*19-20)*stride+offset+std::min(n_capture,max_capture)]=1.;
            }else{
                feature[(x+y*19-20)*stride+offset]=1.;
            }
        }
    }foreach_free_point_end;
}

void GoBoard::extract_feature_dragon_safety(float* feature,int stride,int offset){
    for (group_t g = 1; g < board_size2(b); g++){
        if (group_at(b , g) == g){
            //enum stone color=board_at(b,g);
//            bool safe=dragon_is_safe(b,g,color);
            bool surround=dragon_is_surrounded(b,g);
            foreach_in_group(b, g) {
                int x=coord_x(c,b);
                int y=coord_y(c,b);
/*                if(safe){
                    feature[(x+y*19-20)*stride+offset]=1.;
                }*/
                if(surround){
                    feature[(x+y*19-20)*stride+offset+1]=1.;
                }
            } foreach_in_group_end;
        }
    }
}


void GoBoard::extract_feature_dragon_lib(float* feature,int stride,int offset){
    for (group_t g = 1; g < board_size2(b); g++){
        if (group_at(b , g) == g){
            enum stone color=board_at(b,g);
            int n_lib=dragon_liberties(b,color,g);
            foreach_in_group(b, g) {
                int x=coord_x(c,b);
                int y=coord_y(c,b);
                feature[(x+y*19-20)*stride+offset]=exp(-0.2*n_lib);
            } foreach_in_group_end;
        }
    };
}

void GoBoard::extract_feature_dragon_mgroup(float* feature,int stride, int offset, int max_history){
    int history_len=history.length();
    int dragon_leader[24];
    int r=std::min(max_history+1,int(history_len/2));
    for(int turn_since=0; turn_since<r; turn_since++){
        int y=history[history_len-turn_since*2-2]-65+1;
        int x=history[history_len-turn_since*2-1]-97+1;
        if(y!=25+1){
            //not pass
            coord_t c=coord_xy(b,y,x);
            dragon_leader[turn_since]=dragon_at(b,c);
        }else{
            dragon_leader[turn_since]=-100;
        }
    }

    unsigned char watermark[board_size2(b) / 8];
    memset(watermark, 0, sizeof(watermark));

    foreach_point(b){
        if(board_at(b,c)==S_NONE or board_at(b,c)==S_OFFBOARD){
            continue;
        }
        if(watermark_get(c) == 0){
            group_t g=group_at(b,c);
            group_t d=dragon_at(b,c);
            foreach_in_group(b, g) {
                watermark_set(c);
            } foreach_in_group_end;
            int j=0;
            for(j=0;j<r;j++){
                if (d==dragon_leader[j]){
                    foreach_in_group(b, g) {
                        int x=coord_x(c,b);
                        int y=coord_y(c,b);
                        feature[(x+y*19-20)*stride+offset+j]=1.;
                    } foreach_in_group_end;
                }
            }
        }
    }foreach_point_end;
}

void GoBoard::extract_feature_lastko(float* feature,int stride,int offset){
    coord_t c=b->last_ko.coord;
    if (is_pass(c) || b->last_ko_age>=4){
        return;
    }
    int x=coord_x(c,b);
    int y=coord_y(c,b);
    feature[(x+y*19-20)*stride+offset]=1.;
    for(int i=0;i<361;i++){
        feature[i*stride+offset+1]=1.;
    }
}

void GoBoard::extract_feature_group(int* feature){
    int id=1;
    for (group_t g = 1; g < board_size2(b); g++){
        if (group_at(b , g) == g){
            foreach_in_group(b, g) {
                if(id<96){
                    int x=coord_x(c,b);
                    int y=coord_y(c,b);
                    feature[x+y*19-20]=id;
                }
            } foreach_in_group_end;
            id++;
        }
    }
}

void GoBoard::extract_feature_dragon_group(int* feature){
    unsigned char watermark[board_size2(b) / 8];
    memset(watermark, 0, sizeof(watermark));


    int dragon_cnt=1;
    coord_t dragon_id[64];

    for (group_t g = 1; g < board_size2(b); g++){
        if (group_at(b , g) == g){
            group_t d=dragon_at(b,g);
            int id;
            for(id=1;id<dragon_cnt;id++){
                if(dragon_id[id]==d){
                    break;
                }
            }
            if(id<64){
                if(id==dragon_cnt){
                    dragon_id[dragon_cnt++]=d;
                }
            }else{
                id=0;
            }
            foreach_in_group(b, g) {
                int x=coord_x(c,b);
                int y=coord_y(c,b);
                feature[x+y*19-20]=id;
            } foreach_in_group_end;
        }
    };
}

void GoBoard::extract_feature_selfatari_size(float* feature, int stride, int offset, int my_color_int, int max_atari){
    enum stone my_color=(my_color_int==COLOR_BLACK?S_BLACK:S_WHITE);
    foreach_free_point(b){
        if(board_is_valid_play_no_suicide(b, my_color, c)){
            int cnt=0;
            with_move(b,c,my_color,{
                    group_t g=group_at(b,c);
                    if(board_group_info(b,g).libs==1){
                        foreach_in_group(b,g){
                            cnt++;
                        }foreach_in_group_end;
                    }
            });
            int x=coord_x(c,b);
            int y=coord_y(c,b);
            feature[(x+y*19-20)*stride+offset+std::min(cnt,max_atari)]=1.;
        }
    }foreach_free_point_end;
}

void GoBoard::extract_feature_lib_aftermove(float* feature,int stride,int offset,int my_color_int, int max_liberty){
    enum stone my_color=(my_color_int==COLOR_BLACK?S_BLACK:S_WHITE);
    foreach_free_point(b){
        int x=coord_x(c,b);
        int y=coord_y(c,b);
        if(board_is_valid_play_no_suicide(b, my_color, c)){
            with_move(b,c,my_color,{
                    group_t g=group_at(b,c);
                    feature[(x+y*19-20)*stride+offset+std::min(board_group_info(b, g).libs,max_liberty)-1]=1.;
            });
        }
    }foreach_free_point_end;
}

//enemy g can be capture
//capturing enemy g with size [1,2,3+] in position
//capturing enemy g is snapback
//enemy g can counter capture if I kenuki
//enemy g can escape by play on lib if I kenuki
//enemy g with 1 lib can not be capture, ko
void GoBoard::extract_pachi_capture(float *feature,int stride, int offset, int my_color_int){
    enum stone my_color=(my_color_int==COLOR_BLACK?S_BLACK:S_WHITE);
    int x,y;
    for (group_t g = 1; g < board_size2(b); g++){
        if (group_at(b , g) == g){
            if(board_group_info(b, g).libs != 1){
                continue;
            }
            enum stone g_color=board_at(b,g);
            if(g_color==my_color){
                continue;
            }
            coord_t capture = board_group_info(b, g).lib[0];
            bool group_can_countercapture=can_countercapture(b,g,NULL,0);
            bool group_cannot_escape=(!can_play_on_lib(b, g, stone_other(my_color)) || is_ladder(b, capture, g, true));

            if(board_is_valid_play_no_suicide(b, my_color, capture)){
                foreach_in_group(b, g) {
                    x=coord_x(c,b);
                    y=coord_y(c,b);
                    //captureable [0]
                    feature[(x+y*19-20)*stride+offset+0]=1.;
                    //enemy group can counter capture, if I kenuki [1]
                    if(group_can_countercapture){
//                        fprintf(stderr, "group %s can escape by counter capture\n", coord2str(c,b));
                        feature[(x+y*19-20)*stride+offset+1]=1.;
                    }
                    //enemy group can escape, if I kenuki [2]
                    if(group_cannot_escape){
//                        fprintf(stderr, "group %s can escape by put on lib\n", coord2str(c,b));
                        feature[(x+y*19-20)*stride+offset+2]=1.;
                    }
                }foreach_in_group_end;
            }else{
                //group with 1lib but not in atari, ko [3]
                //fprintf(stderr, "group is ko fight %s\n",coord2str(g,b));
                foreach_in_group(b, g) {
                    int x=coord_x(c,b);
                    int y=coord_y(c,b);
                    //captureable [0]
                    feature[(x+y*19-20)*stride+offset+3]=1.;
                }foreach_in_group_end;
            }
        }
    }
}

// is mygroup g can counter capture
// is mygroup g can escape

// No need to mark the group in atari, since it is equal to lib=1
void GoBoard::extract_pachi_selfatari(float *feature,int stride, int offset, int my_color_int){
    enum stone my_color=(my_color_int==COLOR_BLACK?S_BLACK:S_WHITE);

    struct move_queue defender;
    defender.moves=0;
    int x,y;
    for (group_t g = 1; g < board_size2(b); g++){
        if (group_at(b , g) == g){
            if(board_group_info(b, g).libs != 1){
                continue;
            }
            enum stone g_color=board_at(b,g);
            if(g_color!=my_color){
                continue;
            }
            coord_t escape = board_group_info(b, g).lib[0];

            bool group_can_countercapture=can_countercapture(b,g,&defender,0);
            bool group_cannot_escape=(!can_play_on_lib(b, g, my_color) || is_ladder(b, escape, g, true));

            foreach_in_group(b, g) {
                x=coord_x(c,b);
                y=coord_y(c,b);
                //my group can countercapture[2]
                if(group_can_countercapture){
                    //board_print(b,stderr);
                    //fprintf(stderr, "selfatari group %s can escape by counter capture\n", coord2str(c,b));
                    feature[(x+y*19-20)*stride+offset+0]=1.;
                }
                //my group can escape[3]
                if(group_cannot_escape){
                    //board_print(b,stderr);
                    //fprintf(stderr, "selfatari group %s cannot escape by put on lib\n", coord2str(c,b));
                    feature[(x+y*19-20)*stride+offset+1]=1.;
                }
            }foreach_in_group_end;
        }
    }
}

//can attack capturing enemy 2lib group
//can attack atari enemy 2lib group
//can defend capturing my 2lib group
//can defend atari my 2lib group
void GoBoard::extract_pachi_2lib(float *feature,int stride, int offset, int my_color_int){
    enum stone my_color=(my_color_int==COLOR_BLACK?S_BLACK:S_WHITE);
    struct move_queue att_cap;
    att_cap.moves=0;
    struct move_queue att_atari;
    att_atari.moves=0;
    struct move_queue def_cap;
    def_cap.moves=0;
    struct move_queue def_atari;
    def_atari.moves=0;

    int x,y;

    for (group_t g = 1; g < board_size2(b); g++){
        if (group_at(b , g) == g){
            if(board_group_info(b, g).libs != 2){
                continue;
            }
            enum stone g_color=board_at(b,g);
            if(g_color!=my_color){
                if(can_capture_2lib_group(b, g, stone_other(my_color), &att_cap, 0)){
//                     board_print(b,stderr);
//                     fprintf(stderr, "capture enemy 2lib %s\n", coord2str(g,b));
                    foreach_in_group(b, g) {
                        x=coord_x(c,b);
                        y=coord_y(c,b);
                        //successfully capture enemy 2lib [0]
                        feature[(x+y*19-20)*stride+offset+0]=1.;
                    }foreach_in_group_end;
                }
                if(miai_2lib(b, g, stone_other(my_color))){
                    unsigned int t=att_atari.moves;
                    can_atari_group(b,g,g_color,my_color,&att_atari,0,true);
                    if(att_atari.moves!=t){
//                        board_print(b,stderr);
//                        fprintf(stderr, "atari enemy 2lib %s\n", coord2str(g,b));
                        foreach_in_group(b, g) {
                            x=coord_x(c,b);
                            y=coord_y(c,b);
                            //successfully atari enemy 2lib [1]
                            feature[(x+y*19-20)*stride+offset+1]=1.;
                        }foreach_in_group_end;
                    }
                }
            }else{
                /* Can we counter-atari another group, if we are the defender? */
                bool semeai_capture_win=false;
                bool semeai_atari_win=false;
                foreach_in_group(b, g) {
                    foreach_neighbor(b, c, {
                            if (board_at(b, c) != stone_other(my_color))
                            continue;
                            group_t g2 = group_at(b, c);
                            if (board_group_info(b, g2).libs == 1 && board_is_valid_play(b, my_color, board_group_info(b, g2).lib[0])) {
                                /* We can capture a neighbor. */
                                mq_add(&def_cap, board_group_info(b, g2).lib[0], 0);
                                mq_add(&def_atari, board_group_info(b, g2).lib[0], 0);
                                semeai_capture_win=true;
                                semeai_atari_win=true;
                                continue;
                            }
                            if (board_group_info(b, g2).libs == 2){
                                if(can_capture_2lib_group(b, g2, stone_other(my_color), &def_cap, 0)){
                                    semeai_capture_win=true;
                                }
                                unsigned int t=def_atari.moves;
                                can_atari_group(b, g2, stone_other(my_color), my_color, &def_atari, 0, true);
                                if(t!=def_atari.moves){
                                    semeai_atari_win=true;
                                }
                            }
                    });
                } foreach_in_group_end;
                if(semeai_capture_win){
//                    board_print(b,stderr);
//                    fprintf(stderr, "defend %s by capturing enemy\n", coord2str(g,b));
                    foreach_in_group(b, g) {
                        x=coord_x(c,b);
                        y=coord_y(c,b);
                        //successfully defend my 2lib capture [2]
                        feature[(x+y*19-20)*stride+offset+2]=1.;
                    }foreach_in_group_end;
                }
                if(semeai_atari_win){
//                    board_print(b,stderr);
//                    fprintf(stderr, "defend %s by atari enemy\n", coord2str(g,b));
                    foreach_in_group(b, g) {
                        x=coord_x(c,b);
                        y=coord_y(c,b);
                        //successfully defend my 2lib atari [3]
                        feature[(x+y*19-20)*stride+offset+3]=1.;
                    }foreach_in_group_end;
                }
            }

        }
    }
}
void GoBoard::extract_feature_dragon_old(float* feature,int stride,int offset){
    unsigned char watermark[board_size2(b) / 8];
    memset(watermark, 0, sizeof(watermark));

    foreach_point(b){
        if(board_at(b,c)==S_NONE or board_at(b,c)==S_OFFBOARD){
            continue;
        }
        if(watermark_get(c) == 0){
            group_t g=group_at(b,c);
            enum stone color=board_at(b,c);
            bool safe=dragon_is_safe(b,g,color);
            bool surround=dragon_is_surrounded(b,c);
            int n_lib=dragon_liberties(b,color,c);
            foreach_in_group(b, g) {
                watermark_set(c);
                int x=coord_x(c,b);
                int y=coord_y(c,b);
                if(safe){
                    feature[(x+y*19-20)*stride+offset]=1.;
                }
                if(surround){
                    feature[(x+y*19-20)*stride+offset+1]=1.;
                }
                if(n_lib<=4){
                    feature[(x+y*19-20)*stride+offset+2]=1.;
                }else{
                    feature[(x+y*19-20)*stride+offset+3]=1.;
                }
            } foreach_in_group_end;
        }
    }foreach_point_end;
}

void GoBoard::extract_feature_ladder_capture_and_cann_escape(float* feature,int stride,int offset,int my_color_int){
    unsigned char watermark[board_size2(b) / 8];
    memset(watermark, 0, sizeof(watermark));
    enum stone my_color=(my_color_int==COLOR_BLACK?S_BLACK:S_WHITE);

    foreach_point(b){
        if(board_at(b,c)==S_NONE or board_at(b,c)==S_OFFBOARD or board_at(b,c)==my_color){
            continue;
        }
        group_t group = group_at(b, c);
        if (board_group_info(b, group).libs == 2 && watermark_get(c)==0){
            foreach_in_group(b,group){
                watermark_set(c);
            }foreach_in_group_end;
            for(int i = 0; i < 2; i++) {
                coord_t chase = board_group_info(b, group).lib[i];
                coord_t escape = board_group_info(b, group).lib[1 - i];
                if (wouldbe_ladder(b, group, escape, chase, board_at(b, group))){
                    int x=coord_x(chase,b);
                    int y=coord_y(chase,b);
                    feature[(x+y*19-20)*stride+offset]=1.;
                }
            }
        }
    }foreach_point_end;

    //check if cannot escape ladder =1
    foreach_free_point(b){
        if(board_is_valid_play_no_suicide(b, my_color, c)){
            group_t atari_neighbor = board_get_atari_neighbor(b, c, my_color);
            if (atari_neighbor && is_ladder(b, c, atari_neighbor, true) &&
                !useful_ladder(b, atari_neighbor)) {
                int x=coord_x(c,b);
                int y=coord_y(c,b);
                feature[(x+y*19-20)*stride+offset+1]=1.;
            }
        }
    }foreach_free_point_end;
}

void GoBoard::extract_feature_turnsince(float* feature,int stride,int offset,int max_history){
    int history_len=history.length();
    for(int turn_since=0; turn_since<std::min(max_history+1,int(history_len/2)); turn_since++){
        int col=history[history_len-turn_since*2-2]-65;
        int row=history[history_len-turn_since*2-1]-97;
        if(col!=25){
            //not pass
            feature[(row*19+col)*stride+offset+turn_since]=1.;
        }
    }
}

void GoBoard::extract_feature_planes(float* feature,int stride,int t, int next_color){
    enum stone my_color=(next_color==COLOR_BLACK?S_BLACK:S_WHITE);
    foreach_point(b){
        enum stone color = board_at(b, c);
        int row=coord_x(c,b)-1;
        int col=coord_y(c,b)-1;
        if (color == my_color)
            feature[(col*19+row)*stride + t*2]=1.;
        else if (color != S_NONE && color != S_OFFBOARD)
            feature[(col*19+row)*stride + t*2 + 1]=1.;
    }foreach_point_end;
}

void GoBoard::extract_feature_next_color(float* feature,int stride,int offset){
    for (int i = 0; i < 361; i++)
        feature[i*stride + 16 + offset] = 1.;
}

void GoBoard::extract_feature_all_planes(float* feature, int stride, int next_color, std::string new_history){
    int total_length = std::min(7, int(new_history.length()/2));
    extract_feature_planes(feature,stride,total_length,next_color);
    for (int i = 0; i < total_length; i++){
        std::string move = new_history.substr(new_history.length() - total_length * 2 + i * 2, 2);
        char hi = move[0];
        char lo = move[1];
        if (hi!='Z') {
            int pos=(hi-'A')+(lo-'a')*19;
            apply_move(pos);
        }else
            apply_move(-1);
        extract_feature_planes(feature,stride,total_length -1 - i,next_color);
    }
}

#undef watermark_get
#undef watermark_set




