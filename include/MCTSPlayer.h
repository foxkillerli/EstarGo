//
// Created by Kaihua Li on 2022/7/4.
//

#ifndef ESTAR_GO_MCTSPLAYER_H
#define ESTAR_GO_MCTSPLAYER_H

#include <atomic>
#include <string>
#include <vector>
#include <thread>
#include <cstring>
#include <random>
#include <chrono>

#include "MCTSTree.h"
//#include "Evaluator.h"
#include "../include/utils/Config.h"
#include "GoBoard.h"
#include "utils/Executor.h"
#include "Symmetry.h"
#include "Latency.h"

/**
 * A player with Monte-Carlo Tree Search that uses a tree policy that gives prior probability of each node and a default policy that estimates the win rate of each node.
 * TODO: Add value network.
 */
class MCTSPlayer {

private:
    NodePtr root;

    Executor *executor;
    Symmetry *symmetry;

    goeval::Evaluator evaluator_;

    NodePtr tmp;

    int virtual_loss; /**< Virtual Loss (Should be larger than 0 when multiple threads are traversing the tree. */
    float c_puct; /**< Exploration constant. Increasing this constant encourages MCTS to explore nodes lower utility score. */

    /*time policy: early+exp*/
    int dynamic_timeout; /**< Time out for each move, break rollout loop when time exceeeded. */
    int dynamic_rollout; /**< Max rollout for each move, break rollout loop when time exceeeded. */
    int time_policy; /** turn on/off flag of time policy **/
    int time_remain; /** time left in second **/
    int rollout_speed; /** rollout speed in second **/
    float f_early;
    int n_thread;
    int dynamic_exp;
    int n_gpu;
    bool test_mode; /** true for sync, false for async **/

    /** loging field **/
    int total_rollout;/** Total rollout in AI's turn*/

    bool use_server_eval;
    bool stop_rollout;

    bool is_uec;
    int time_out_ms; /**< Time out for each move, break rollout loop when time exceeeded. */
    int stage_0_safe_time_s;
    int stage_150_safe_time_s;
    int quick_play_move_num;
    int quick_play_move_time_out_ms;
    int over_using_thr_time_out_ms;
    int under_using_thr_time_out_ms;
    int under_using_time_out_ms;

    volatile bool _mIsExitRollout;
    volatile bool _mIsRolloutFinished;

    int n_speed;
    int op_action;  /** last op action **/

    std::vector<std::future<void>> search_thread_vec;

    std::vector<std::string> policy;
    std::vector<float> win;
    std::vector<float> win_rate;
    std::vector<float> tromp_taylor;
    std::vector<std::string> history;
    double cross_entropy;
    std::vector<double> cross_entropy_list;

    typedef std::pair<int, int> NPAIR;

    static bool compare(const NPAIR a, const NPAIR b){
        return a.first > b.first;
    }


public:
    GoBoard *root_board;

    static const int INIT_ROLLOUT_SPEED=850;  // This is the init rollout speed for each async send thread (equals with GPU num)
    MCTSPlayer(Executor *exe, std::string history = "") {
        executor=exe;
        symmetry = new Symmetry();
        srand(time(NULL));
        root_board = new GoBoard(7.5, 19);
        root_board->apply_history(history);
        //root_board->apply_history("DdPdCpPpEqCjNcQfPgQgChCmDlClDjDkEkCkDiElFkJdHcIePcQcQdOcPbOdObRdNdMbLcLbKcJcKbJbOeQeKeKaPeQdNbGdGcFdEbDeCeCdDcCfBeDfBfIgKgIiPhQiFfFgGgGhFhEgGfHgGiHhEeEfNqLqQqPqQpPoPrOrQrOqQnGqKpKqJpJqIpIqBoHpEmDoFlCoBnDpCqDqCnDnDmCrBrDrBsKdLdLfKfNgEhFeCgEdDgEeOgNiNfMgMfLgLePfOfKiPiQjPjNkQkRkQhRhImQlPkRmRnKmPlOmJnGnHlLoHdHeLpMpLnKnKoIoJoMoKlJmInJlJkKkIlLlLmKlNjMjNlOlOkOjFoFpGoHoEoEpHnEnFnIkHkIjOnPnPmNmQmRlNhMhOiNjLhMiOsNsRbQbQaSnSmSlPsNrRgRiRfReRcSgSdSfKhSpLiLjJiJjSqRqSrRrRpSsSqSoRsJhSrHbGbHaGaIcMcLaCsDsScHjGjFcFbGpGmNaOaJaEcGeJeIdIfHfJfSeOhMaBqJgRoSmQoHiCcMmMeMnRaOoPaApBpAqArBmAmAlBkAnAoIaAmBjBhAnEaAiAmAkBiErAhSjAnMqAjHrBlFsCiFiEiGkGlFjEjFiFjGgCaLrSaJrApKsDkCmClNpCjNnBmMlAgGsBdFqBbIsAcMsAaRgDbLmAkLkAeFfRqFrSiSh");
        root = NodePtr(new Node());
        root->depth=root_board->get_total_moves();
        total_rollout = 0;
        time_remain = 600;
        rollout_speed = INIT_ROLLOUT_SPEED * Config::asyn_once_send();
        rollout_speed = rollout_speed / Config::search_threads();
        virtual_loss = Config::virtual_loss();
        c_puct = Config::c_puct();
        time_policy = Config::is_time_policy();
        time_out_ms = Config::time_out_ms();
        stage_0_safe_time_s = Config::stage_0_safe_time_s();
        stage_150_safe_time_s = Config::stage_150_safe_time_s();
        quick_play_move_num = Config::quick_play_move_num();
        quick_play_move_time_out_ms = Config::quick_play_move_time_out_ms();
        over_using_thr_time_out_ms = Config::over_using_thr_time_out_ms();
        under_using_thr_time_out_ms = Config::under_using_thr_time_out_ms();
        under_using_time_out_ms = Config::under_using_time_out_ms();
        //int start_gpu_id=config.get_int("eval_start_id");
        //int end_gpu_id=config.get_int("eval_end_id");
        f_early=2;

        use_server_eval=Config::use_server_evaluator();

        _mIsExitRollout = false;
        _mIsRolloutFinished = true;

        n_speed = rollout_speed;
        op_action = 0;

        stop_rollout = false;
    }

    ~MCTSPlayer() {
        stop_pondering();
        delete root_board;
        delete symmetry;
        root=NULL;
    }

    /**
     * Returns the color of next player.
     */
    int get_color() {
        return root_board->get_color();
    }

    std::string gen_apply_move();
    void play(char *s_pos);
    void rollout(int max);
    void est_timeout_rollout();
    int get_rollout_num();
    void server_evaluate(GoBoard& cur_board, NodePtr leaf, int max,std::atomic_int& rollout_in_turn, Latency& eval_latency, std::atomic<long>& pending_reply);
    int on_server_rollout_loop_over(std::atomic<long>& pending_reply);
    void cal_rollout_speed(int rollout_in_turn, int server_eval_cnt, long cost_time);

    void time_left(int time_left){
        this->time_remain=time_left;
    }
    float get_final_score(){
        return root_board->get_score_black();
    }
    void stop_pondering(){
        if(!_mIsExitRollout){
            _mIsExitRollout=true;
            /*while(!_mIsRolloutFinished){
              std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }*/
            for (size_t i = 0; i < search_thread_vec.size(); ++i) {
                search_thread_vec[i].wait();
            }
            search_thread_vec.clear();
        }
        _mIsExitRollout=false;
    }

    void start_pondering(){
        if (stop_rollout) return;
        for (int i = 0; i < Config::search_threads(); ++i) {
            std::future<void> f = executor->commit(&MCTSPlayer::rollout, this,-1);
            search_thread_vec.push_back(std::move(f));
        }
    }

    std::string get_policy(int index){
        return policy[index];
    }

    float get_win(int index){
        return win[index];
    }

    float get_win_rate(int index){
        return win_rate[index];
    }

    int get_tromp_taylor(int index){
        return tromp_taylor[index];
    }

    std::string get_history(int index){
        return history[index];
    }

    float get_cross_entropy(){
        return cross_entropy;
    }

    float get_cross_entropy_list(int index){
        return cross_entropy_list[index];
    }
};

#endif //ESTAR_GO_MCTSPLAYER_H