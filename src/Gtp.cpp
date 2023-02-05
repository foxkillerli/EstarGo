//
// Created by Kaihua Li on 2022/7/3.
//

#include "../include/Gtp.h"
#include <cstring>
#include <string>
#include <chrono>
#include <thread>

using std::endl;
using std::string;

/**
 * Sets the AI player as the one specified in config file.
 */
void GTP::initialize(string history) {
    mcts_player = new MCTSPlayer(executor, history);
    my_name = Config::gtp_name();
    my_version = Config::gtp_version();
    is_pondering = Config::is_pondering();
    is_lcmd_genmove = false;
}

// helper functions for switching string
typedef std::uint64_t hash_t;
constexpr hash_t prime = 0x100000001B3ull;
constexpr hash_t basis = 0xCBF29CE484222325ull;
hash_t hash_(char const* str)
{
    hash_t ret{basis};
    while(*str){
        ret ^= *str;
        ret *= prime;
        str++;
    }
    return ret;
}
constexpr hash_t hash_compile_time(char const* str, hash_t last_value = basis)
{
    return *str ? hash_compile_time(str+1, (*str ^ last_value) * prime) : last_value;
}
// helper function end

void GTP::process_cmd_play(char command[]) {
    mcts_player->stop_pondering();
    LOG_INFO("pondering stopped.");
    char pos[5];
    char col[5];
    sscanf(command, "%*s %s %s", col, pos);
    mcts_player->play(pos);
    gtp_print("");
    if (strcasecmp(pos,"RESIGN")==0){
        if (mcts_player) {
            delete mcts_player;
            mcts_player = 0;
        }
    }
    is_lcmd_genmove = false;
    //TODO: we might start pondering here
}

void GTP::process_cmd_genmove(char command[]) {
    mcts_player->stop_pondering();
    LOG_INFO("pondering stopped.");
    if(is_lcmd_genmove){
        char s[10]="PASS";
        mcts_player->play(s);
        LOG_INFO("opponent implicit pass");
        //mcts_player->stop_pondering();
    }
    char color[5];
    sscanf(command, "%*s %s", color);
    string s_pos = mcts_player->gen_apply_move();
    gtp_print(s_pos.c_str());
    if(is_pondering && strcasecmp(s_pos.c_str(),"resign")!=0){
        mcts_player->start_pondering();
    }
    is_lcmd_genmove=true;
}

void GTP::process_cmd_showboard(char command[]){
    gtp_print(mcts_player->root_board->print_board_buffer().c_str());
    //gtp_print("");
}

void GTP::process_cmd_timeleft(char command[]) {
    char color[5], time[4], stone[4];
    sscanf(command, "%*s %s %s %s",color, time, stone );
    string color_received(1, color[0]);
    string color_str_upper = to_upper_string(color_received);
    int n_player_color = GoBoard::COLOR_BLACK;
    if (color_str_upper == "W") {
        n_player_color = GoBoard::COLOR_WHITE;
    }
    if (n_player_color == mcts_player->get_color()) {
        mcts_player->time_left(atoi(time));
        LOG_INFO("Main Time Left: " << time << "s, Byo Left: " << stone);
    }
    gtp_print("");
}

void GTP::process_cmd_quit(char command[]) {
    if (mcts_player!=NULL) {
        mcts_player->stop_pondering();
        delete mcts_player;
        mcts_player = NULL;
    }
    gtp_print("");
}

void GTP::process_cmd_name(char command[]) {
    gtp_print(my_name.c_str());
}

void GTP::process_cmd_protocolversion(char command[]) {
    gtp_print("2.0");
}

void GTP::process_cmd_version(char command[]) {
    gtp_print(my_version.c_str());
}

void GTP::process_cmd_listcommands(char command[]) {
    std::stringstream sstream;
    sstream << GTP_CMD_PLAY << endl
            << GTP_CMD_GENMOVE << endl
            << GTP_CMD_TIME_LEFT << endl
            << GTP_CMD_QUIT << endl
            << GTP_CMD_NAME << endl
            << GTP_CMD_PROTOCOL_VERSION << endl
            << GTP_CMD_VERSION << endl
            << GTP_CMD_LIST_COMMANDS << endl
            << GTP_CMD_BOARDSIZE << endl
            << GTP_CMD_CLEAR_BOARD << endl
            << GTP_CMD_KOMI << endl
            << GTP_FINAL_SCORE << endl
            << GTP_TIME_SETTINGS << endl
            << GTP_SHOW_BOARD << endl;
    gtp_print(sstream.str().c_str());
}

void GTP::process_cmd_boardsize(char command[]) {
    gtp_print("");
    LOG_WARN("By pass board size setting, it must be 19");
}

void GTP::process_cmd_clearboard(char command[]) {
    if (mcts_player) {
        mcts_player->stop_pondering();
        //delete mcts_player;
        //mcts_player = NULL;
        std::swap(mcts_player, tmp_mcts_player);
        mcts_player = NULL;
        std::thread recycle = std::thread([&](){
            delete tmp_mcts_player;
            tmp_mcts_player = NULL;
        });
        recycle.detach();
    }
    LOG_INFO("mcts_player deleted");
    initialize();
    gtp_print("");
    is_lcmd_genmove = false;
    LOG_INFO("clearboard finished.");
}

void GTP::process_cmd_komi(char command[]) {
    gtp_print("");
    LOG_WARN("By pass komi setting, it must be 7.5");
}

void GTP::process_cmd_finalscore(char command[]) {
    float w=mcts_player->get_final_score();
    char final_score[20];
    if(w>0){
        sprintf(final_score,"W+%.1f",w);
    }else{
        sprintf(final_score,"B+%.1f",-w);
    }
    gtp_print(final_score);
    LOG_INFO("final_score" << final_score);
}

void GTP::process_cmd_timesettings(char command[]) {
    gtp_print("");
    LOG_WARN("By pass timesetting command");
}

bool GTP::process_cmd(char command[]) {
    bool is_quit = false;
    char cmd_name[100];
    sscanf(command, "%s", cmd_name);
    LOG_INFO("received command: " << command);
    switch(hash_(cmd_name)) {
        case hash_compile_time(GTP_CMD_PLAY):
            process_cmd_play(command);
            break;
        case hash_compile_time(GTP_CMD_GENMOVE):
            process_cmd_genmove(command);
            break;
        case hash_compile_time(GTP_CMD_TIME_LEFT):
            process_cmd_timeleft(command);
            break;
        case hash_compile_time(GTP_CMD_QUIT):
            process_cmd_quit(command);
            is_quit = true;
            break;
        case hash_compile_time(GTP_CMD_NAME):
            process_cmd_name(command);
            break;
        case hash_compile_time(GTP_CMD_PROTOCOL_VERSION):
            process_cmd_protocolversion(command);
            break;
        case hash_compile_time(GTP_CMD_VERSION):
            process_cmd_version(command);
            break;
        case hash_compile_time(GTP_CMD_LIST_COMMANDS):
            process_cmd_listcommands(command);
            break;
        case hash_compile_time(GTP_CMD_BOARDSIZE):
            process_cmd_boardsize(command);
            break;
        case hash_compile_time(GTP_CMD_CLEAR_BOARD):
            process_cmd_clearboard(command);
            break;
        case hash_compile_time(GTP_CMD_KOMI):
            process_cmd_komi(command);
            break;
        case hash_compile_time(GTP_FINAL_SCORE):
            process_cmd_finalscore(command);
            break;
        case hash_compile_time(GTP_TIME_SETTINGS):
            process_cmd_timesettings(command);
            break;
        case hash_compile_time(GTP_SHOW_BOARD):
            process_cmd_showboard(command);
            break;
        default:
            gtp_print_err("does not support this command.");
    }
    return is_quit;
}

void GTP::run() {
    LOG_INFO("Start a new GTP game.");
    char command[1024];
    while(true) {
        while(fgets(command, sizeof(command),stdin)==NULL){
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        bool quit = process_cmd(command);
        if (quit) {
            break;
        }
    }
}