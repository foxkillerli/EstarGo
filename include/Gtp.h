//
// Created by Kaihua Li on 2022/7/3.
//


#ifndef GO_AGENT_MCTS_GTP_H
#define GO_AGENT_MCTS_GTP_H

#include <cstdio>
#include <string>
#include "GoBoard.h"
#include "Config.h"
#include "utils/Executor.h"
#include "MCTSPlayer.h"
#include "utils/Logger.h"

//#include <grpc++/grpc++.h>

/**
 * A class that implements GTP Protocol
 */
class GTP {
public:
    GTP() {
        mctsplayer = 0;
        tmp_mctsplayer = 0;
        executor=new Executor(Config::n_thread());
        is_pondering=0;
    }

    void initialize(std::string history = "");
    void run();

private:
    /**
     * Print message $s in GTP format.
     */
    void gtp_print(const char *s) {
        printf("= %s\n\n", s);
        fflush(stdout);
    }

    /**
     * Print error message $s in GTP format.
     */
    void gtp_print_err(const char *s) {
        printf("? %s\n\n", s);
        fflush(stdout);
    }
    bool process_cmd(char command[]);
    void process_cmd_play(char command[]);
    void process_cmd_genmove(char command[]);
    void process_cmd_timeleft(char command[]);
    void process_cmd_quit(char command[]);
    void process_cmd_name(char command[]);
    void process_cmd_protocolversion(char command[]);
    void process_cmd_version(char command[]);
    void process_cmd_listcommands(char command[]);
    void process_cmd_boardsize(char command[]);
    void process_cmd_clearboard(char command[]);
    void process_cmd_komi(char command[]);
    void process_cmd_finalscore(char command[]);
    void process_cmd_timesettings(char command[]);
    void process_cmd_showboard(char command[]);

private:
    static constexpr const char* GTP_CMD_PLAY = "play";
    static constexpr const char* GTP_CMD_GENMOVE = "genmove";
    static constexpr const char* GTP_CMD_TIME_LEFT = "time_left";
    static constexpr const char* GTP_CMD_QUIT = "quit";
    static constexpr const char* GTP_CMD_NAME = "name";
    static constexpr const char* GTP_CMD_PROTOCOL_VERSION = "protocol_version";
    static constexpr const char* GTP_CMD_VERSION = "version";
    static constexpr const char* GTP_CMD_LIST_COMMANDS = "list_commands";
    static constexpr const char* GTP_CMD_BOARDSIZE = "boardsize";
    static constexpr const char* GTP_CMD_CLEAR_BOARD = "clear_board";
    static constexpr const char* GTP_CMD_KOMI = "komi";
    static constexpr const char* GTP_FINAL_SCORE = "final_score";
    static constexpr const char* GTP_TIME_SETTINGS = "time_settings";
    static constexpr const char* GTP_SHOW_BOARD = "showboard";
    static constexpr const char* GTP_CGOS_OPPONENT_NAME = "cgos-opponent_name";
    static constexpr const char* GTP_CGOS_GAMEOVER = "cgos-gameover";

private:
    MCTSPlayer *mctsplayer; /**< AI player */
    MCTSPlayer *tmp_mctsplayer; /**< tmp AI player */
    Executor *executor;
    std::string my_name;
    std::string my_version;
    int is_pondering;
    bool is_lcmd_genmove;
};
#endif //GO_AGENT_MCTS_GTP_H
