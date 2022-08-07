//
// Created by Kaihua Li on 2022/7/4.
//

#include "../include/MCTSPlayer.h"
#include <cassert>
#include <omp.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <future>
#include <functional>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <vector>
#include <cstdio>
#include "../include/Latency.h"
#include "../include/Utils.h"
#include "../include/utils/Logger.h"
#include "../include/utils/Config.h"
#include "../include/Eval_Def.h"

using namespace std;

/**
 * Semantics:
 *   The root is set to its son, whose state is exactly the same as current board.
 *   All other siblings are discarded.
 */
void MCTSPlayer::play(char *s_pos) {
    assert(root != NULL);

    if (stop_rollout) {
        LOG_INFO("Stop rollout, play pass.");
        return;
    }

    int id = -1;

    bool succ=false;
    if(strcasecmp(s_pos, "resign")==0){
        succ=root_board->apply_move(GoBoard::POS_PASS);
    }else{
        if (strcasecmp(s_pos, "pass")==0){
            succ=root_board->apply_move(GoBoard::POS_PASS);
            op_action=-1;
        }else{
            s_pos[0]=toupper(s_pos[0]);
            int row = atoi(s_pos + 1) - 1;
            int col = s_pos[0] - 'A' - (s_pos[0] > 'I');
            succ=root_board->apply_move(row*19+col);
            op_action=row*19+col;
        }
        if (root->nexts != NULL) {
            vector<NodePtr> &nexts = *(root->nexts);
            for (uint i = 0; i < nexts.size(); ++i){
                if (nexts[i]->edge->action == op_action) {
                    id = i;
                    break;
                }
            }
        }
    }
    if (!succ) {
        LOG_FATAL("Invalid Move met: " << s_pos);
        LOG_FATAL(root_board->print_board_buffer());
        exit(1);
    }

    if (id < 0) {
        /* opponent play an un-evaluated move, create new node.*/
        //root = NodePtr(new Node());
        tmp = NodePtr(new Node());
        root.swap(tmp);
        std::thread recycle = std::thread([&](){
            tmp.reset();
        });
        recycle.detach();
        LOG_INFO("Opponent play unevaluated move:" << s_pos);
        root->depth=root_board->get_total_moves();
    } else {
        vector<NodePtr> &nexts = *(root->nexts);
        int row=nexts[id]->edge->action/19;
        int col=nexts[id]->edge->action%19;
        float w=float(nexts[id]->edge->w);
        int n=float(nexts[id]->edge->n);
        float win_rate = ((w+n)/2)/(n+1e-8);
        char buf[1024];
        snprintf(buf, 1024, "Opponent play :%c%d [n:%d w:%.3f pri:%.3f val:%.3f win:%.3f]", 'A' + col + (col >= 'I' - 'A'), row + 1, n,w/n,nexts[id]->edge->p,nexts[id]->value_pred,win_rate);
        LOG_INFO(buf);
        //root = nexts[id];
        //root->father.reset();
        root.swap(nexts[id]);
        std::thread recycle = std::thread([&](){
            nexts[id].reset();
            root->father.reset();
        });
        recycle.detach();
    }
    dump_best_move(root,0,100*n_gpu);
    LOG_INFO(root_board->print_board_buffer());
    LOG_INFO("history:" << root_board->get_history());
}

void MCTSPlayer::est_timeout_rollout(){
    int n_mymove=root_board->get_total_moves()/2;
    if(time_policy==0){
        dynamic_timeout=time_out_ms;
    }else if(is_uec && n_mymove>=200){
        dynamic_timeout=0;
        stop_rollout=true;
    }else if(n_mymove>=150){
        /**Still can't avoid exceeding time for opponents who don't resign**/
        int safe_time=stage_150_safe_time_s;
        int my_remain_move=is_uec?(200-n_mymove):100;
        while (time_remain<=safe_time) safe_time=safe_time/2;
        dynamic_timeout=(time_remain-safe_time)*1000/my_remain_move;
    }else if(n_mymove>=quick_play_move_num){
        int safe_time=stage_0_safe_time_s;
        dynamic_timeout=static_cast<int>((time_remain-safe_time)*1000.0/(50+(n_mymove>50?0:50-n_mymove))); /** ERICA-BASELINE, Enhanced formula**/
        /**Don't use f_early if no enough time for later moves in case opponent plays very fast or weirdly**/
        if (dynamic_timeout > over_using_thr_time_out_ms) {
            dynamic_timeout = static_cast<int>(f_early*dynamic_timeout);
        } else if(dynamic_timeout < under_using_thr_time_out_ms) {
            dynamic_timeout = 1000;
        }
    }else{
        dynamic_timeout=quick_play_move_time_out_ms;
    }
    dynamic_rollout=dynamic_timeout*rollout_speed/1000;

    char buf[1024];
    snprintf(buf, 1024, "time_remain=%ds, cnt_step=%d time_to_use=%dms, rollout_speed=%d roll/s, dynamic_rollout=%d", time_remain,n_mymove,dynamic_timeout,rollout_speed,dynamic_rollout);
    LOG_INFO(buf);
}


void MCTSPlayer::rollout(int max){
    if (root->depth > 0 && root->edge != NULL) {
        LOG_INFO("[stat] root-depth=" << root->depth << ", root has been rollout time=" << root->edge->n);
    }

    long start_time=gettime();
    _mIsRolloutFinished = false;
    bool exit_flag = false;
    atomic_int rollout_in_turn{0};

    Latency valid_rollout_latency;
    Latency valid_push_latency;
    Latency invalid_rollout_latency;
    Latency invalid_push_latency;
    Latency select_depth_latency;
    Latency visit_num_latency;
    Latency score_num_latency;
    Latency eval_latency;

    if (max == 1){
        est_timeout_rollout();
    }
    else {
        dynamic_timeout = 30000;
    }

    long rollout_start_time=getutime();
    std::atomic<long> pending_reply{0};
    while(true) {
        if(exit_flag){
            break;
        }
        if (gettime() - start_time > dynamic_timeout || rollout_in_turn >= dynamic_rollout) {
            exit_flag = true;
            continue;
        }
        exit_flag=_mIsExitRollout;
        /* early exit of time policy*/
        if(time_policy == 1 && max == 1){
            int max_n, sec_n;
            max_n = sec_n = -1;
            if (root != NULL && root->nexts != NULL) {
                for (int i = 0; i < (int)root->nexts->size(); i++) {
                    int cur_n = root->nexts->at(i)->edge->n;
                    if (max_n < cur_n) {
                        sec_n = max_n;
                        max_n = cur_n;
                    }else if (cur_n > sec_n) {
                        sec_n = cur_n;
                    }
                }
            }
            int left_rollout = dynamic_rollout - rollout_in_turn;
            if (root->total_n>100 && (left_rollout/f_early < max_n - sec_n)) {
                char buf[1024];
                snprintf(buf, 1024, "Early exit, max_nr=%d, sec_nr=%d, left_rollout=%d", max_n,sec_n,left_rollout);
                LOG_INFO(buf);
                exit_flag = true;
                continue;
            }
        }
        if(root->is_terminate){
            if (Config::selfplay() == 1) LOG_INFO("selfplay terminate error");
            exit_flag=true;
            LOG_INFO("Early exit, root is terminate");
            continue;
        }

        //wait for root expantion
        while(root->rq_sent && !root->eval_ready){
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }

        // rollout_in_turn++;
        GoBoard cur_board(*root_board);
        NodePtr leaf;

        rollout_start_time=getutime();
        int select_depth = 0;
        int visit_num=0;
        int score_num=0;
        leaf = mcts_select_path_root(root, virtual_loss, c_puct,max,&cur_board,dynamic_rollout-rollout_in_turn, select_depth, visit_num, score_num);
        select_depth_latency.Record(select_depth);
        visit_num_latency.Record(visit_num);
        score_num_latency.Record(score_num);
        long before_evaluate_time=getutime();

        if (cur_board.get_board_repetition()) {
            leaf->is_repetition = true;
            leaf->is_repetition_checked = true;
            continue;
        } else {
            leaf->is_repetition = false;
            leaf->is_repetition_checked = true;
        }

        if(leaf->result != 0 || leaf->is_terminate){
            rollout_in_turn++;
            mcts_back_update(leaf, virtual_loss, leaf->result, 0);
            continue;
        }

        if(!leaf->rq_sent) {
            valid_rollout_latency.Record(before_evaluate_time - rollout_start_time);
        } else {
            invalid_rollout_latency.Record(before_evaluate_time - rollout_start_time);
        }


        server_evaluate(cur_board, leaf, max, rollout_in_turn, eval_latency, pending_reply);

        long after_evaluate_time=getutime();
        if(leaf->rq_sent) {
            valid_push_latency.Record(after_evaluate_time - before_evaluate_time);
        } else {
            invalid_push_latency.Record(after_evaluate_time - before_evaluate_time);
        }
    }

    long wait_request_over = gettime();
    int server_eval_cnt = 0;

    server_eval_cnt = on_server_rollout_loop_over(pending_reply);

    LOG_INFO("wait request over time :" << (gettime() - wait_request_over));

    long cost_time = gettime() - start_time;
    if(max==1){
        cal_rollout_speed(rollout_in_turn, server_eval_cnt, cost_time);
    }

    LOG_INFO("rollout finished");
    LOG_INFO("valid rollout latency=" << valid_rollout_latency.get_average() << " max latency=" << valid_rollout_latency.get_max_latency());
    LOG_INFO("valid rollout_cnt =" << valid_rollout_latency.get_record_count());
    LOG_INFO("invalid rollout latency=" << invalid_rollout_latency.get_average() << " max latency=" << invalid_rollout_latency.get_max_latency());
    LOG_INFO("invalid rollout_cnt =" << invalid_rollout_latency.get_record_count());
    LOG_INFO("select depth average=" << select_depth_latency.get_average() << " max select depth=" << select_depth_latency.get_max_latency());
    LOG_INFO("visit num average=" << visit_num_latency.get_average() << " max visit num=" << visit_num_latency.get_max_latency());
    LOG_INFO("score num average=" << score_num_latency.get_average() << " max score num=" << score_num_latency.get_max_latency());
    LOG_INFO("valid push latency=" << valid_push_latency.get_average() << " max latency=" << valid_push_latency.get_max_latency());
    LOG_INFO("valid push_cnt =" << valid_push_latency.get_record_count());
    LOG_INFO("invalid push latency=" << invalid_push_latency.get_average() << " max latency=" << invalid_push_latency.get_max_latency());
    LOG_INFO("invalid push_cnt =" << invalid_push_latency.get_record_count());
    LOG_INFO("average latency=" << eval_latency.get_average() << " max latency=" << eval_latency.get_max_latency());
    LOG_INFO("eval_cnt =" << eval_latency.get_record_count());
    LOG_INFO("[async ] step time:" << gettime() - start_time);
    eval_latency.Clear();

    _mIsRolloutFinished = true;
}

void MCTSPlayer::server_evaluate(GoBoard& cur_board, NodePtr leaf, int max, atomic_int& rollout_in_turn, Latency& eval_latency, std::atomic<long>& pending_reply) {
    int root_color = root_board->get_color();
    int leaf_color = cur_board.get_color();
    if (!leaf->rq_sent){
        rollout_in_turn++;
        leaf->rq_sent=true;
        int min_max=max;
        if(leaf_color!=root_color){
            min_max*=-1;
        }
        float tromp_taylor = 0;
        if(leaf->depth>300){
            int temp_min_max=max;
            if(root_color==GoBoard::COLOR_WHITE){
                temp_min_max*=-1;
            }
            tromp_taylor= (cur_board.get_score_black()*temp_min_max>0) ? 1: -1;
        }
        long st = getutime();
        int vl = virtual_loss;
        evaluator_.PolicyAndValueEval(&cur_board,
                                      [min_max, leaf, st, tromp_taylor, &eval_latency, vl](bool ok, float value_score, std::vector<PolicyItem>& policy_list) mutable {
                                          eval_latency.Record(getutime() - st);
                                          std::sort(policy_list.begin(), policy_list.end(), [](PolicyItem a, PolicyItem b) {
                                              return a.prob > b.prob;
                                          });
                                          float score_value = value_score*min_max;
                                          leaf->value_pred = score_value;
                                          ChildrenVecPtr nexts = ChildrenVecPtr(new std::vector<NodePtr>());
                                          for (uint i = 0;i < policy_list.size(); ++i) {
                                              int pos = policy_list[i].pos;
                                              float prob = policy_list[i].prob;
                                              if (pos!=GoBoard::POS_PASS && prob < Config::min_prob()) continue;
                                              EdgePtr edge = EdgePtr(new Edge(pos, prob, nexts->size()));
                                              NodePtr node = NodePtr(new Node(leaf, edge, nexts->size()));
                                              node->depth = leaf->depth+1;
                                              if (pos == GoBoard::POS_PASS && leaf->depth > 300 && leaf->edge != nullptr && leaf->edge->action == GoBoard::POS_PASS) {
                                                  node->result=tromp_taylor;
                                              }
                                              nexts->push_back(node);
                                          }
                                          leaf->nexts = nexts;
                                          __asm__("mfence");
                                          leaf->eval_ready = true;
                                          mcts_back_update(leaf, vl, score_value, leaf->invalid_visit);
                                      });
    }else{
        leaf->invalid_visit++;
    }
}


int MCTSPlayer::on_server_rollout_loop_over(std::atomic<long>& pending_reply) {
    int server_eval_cnt = 0;
    while (evaluator_.get_eval().get_pending() > 0) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
    LOG_INFO("resend_times:" << evaluator_.get_eval().get_resend_times());
    evaluator_.get_eval().clear_resend_times();
    std::vector<Latency*>& latencys = evaluator_.get_eval().get_latencys();
    for(size_t i=0; i<latencys.size(); ++i) {
        Latency& request_latency = *latencys[i];
        LOG_INFO("request average latency of i:" << i << " = " << request_latency.get_average() << " max latency=" << request_latency.get_max_latency() << "  latency_count:" << request_latency.get_record_count());
        server_eval_cnt += request_latency.get_record_count() * 4;
        request_latency.Clear();
    }

    std::vector<Latency*>& dummy_latencys = evaluator_.get_eval().get_dummy_latencys();
    for(size_t i=0; i<dummy_latencys.size(); ++i) {
        Latency& dummy_request_latency = *dummy_latencys[i];
        LOG_INFO("request average dummy latency of i:" << i << " = " << dummy_request_latency.get_average() << " max dummy latency=" << dummy_request_latency.get_max_latency() << "  latency_count:" << dummy_request_latency.get_record_count());
        dummy_request_latency.Clear();
    }

    return server_eval_cnt;
}

void MCTSPlayer::cal_rollout_speed(int rollout_in_turn, int server_eval_cnt, long cost_time) {
    int ignore_rollout_num = 400;
    if(Config::use_server_evaluator()) {
        ignore_rollout_num = 400 * Config::proxy_number();
        rollout_in_turn = server_eval_cnt;
    }
    LOG_INFO("rollout in turn:" << rollout_in_turn);
    total_rollout+=rollout_in_turn;
    if (rollout_in_turn > ignore_rollout_num) {
        n_speed=(int)(rollout_in_turn/((float)(cost_time)/1000+0.01));
    }
    if(rollout_speed > 200 && rollout_in_turn > ignore_rollout_num){
        rollout_speed=rollout_speed*0.9+n_speed*0.1;
    }
    LOG_INFO("rollout_speed: " << rollout_speed << "n_speed: " << n_speed);
    LOG_INFO("rollout_in_turn=" << rollout_in_turn << " avg_rollout=" << int(total_rollout / (root_board->get_total_moves()/2+1)));
}

/**
 * Semantics:
 *   Execute n_rollout number of MCTS simulations (Selection, Evaluation, Backup and Expansion)
 *   Find the best move of current player.
 *   Apply the move to current GO board. (That will change the internal state of board variable)
 *   Return the best move to GTP.
 */
string MCTSPlayer::gen_apply_move()
{
    assert(search_thread_vec.size() == 0);

    if (stop_rollout) {
        LOG_INFO("Stop rollout, play pass.");
        return string("pass");
    }

    for (int i = 0;i < Config::n_thread(); ++i) {
        future<void> f = executor->commit(&MCTSPlayer::rollout, this, 1);
        search_thread_vec.push_back(std::move(f));
    }
    for (int i = 0; i < Config::n_thread(); ++i) {
        search_thread_vec[i].wait();
    }
    search_thread_vec.clear();
    /*
    std::future<void> future=executor->commit(&MCTSPlayer::rollout, this,1);
    future.wait();
    */

    dump_best_move(root,0,100*n_gpu);

#if 0
    TreeInfo info;
  collect_tree_info(root,0,&info);
  LOG_INFO("[treeinfo] max depth=" << info.max_depth << ", avg depth=" << info.avg_depth *1.0 / info.node_number << ", nodes number=" << info.node_number);
  LOG_INFO("[treeinfo] non leaf nodes=" << info.non_leaf_node_number << ", avg width=" << info.avg_width * 1.0 / info.non_leaf_node_number);

  for(int i=0;i<info.max_depth;i++){
    LOG_INFO("[treeinfo] depth=" << i << " n_node=" << info.width[i]);
  }
#endif

#if 0
    char history_buf[2048];
    strcpy(history_buf,root_board->get_history().c_str());
    dump_all_move(history_buf,strlen(history_buf),root, root_board->get_color()==GoBoard::COLOR_BLACK ? 1 : -1);
#endif

    MCTSPair res = mcts_select_action(root, root_board, op_action==GoBoard::POS_PASS);

    char s_pos[10];
    if (res.action == -100){
        // nearly to loss, resign
        root.reset(); // any action after play resign will raise assert failure
        strcpy(s_pos,"resign");
    } else {
        if(res.action == GoBoard::POS_PASS){
            strcpy(s_pos,"pass");
        }else{
            int row = res.action / 19;
            int col = res.action % 19;
            sprintf(s_pos, "%c%d", 'A' + col + (col >= 'I' - 'A'), row + 1);
        }
        //root = res.node;
        root.swap(res.node);
        std::thread recycle = std::thread([&](){
            res.node.reset();
        });
        recycle.detach();
        bool succ = root_board->apply_move(res.action);
        if (!succ) {
            LOG_FATAL("Invalid Move met: " << res.action);
            LOG_FATAL(root_board->print_board_buffer());
            exit(1);
        }
    }
    LOG_INFO(root_board->print_board_buffer());
    LOG_INFO("history:" << root_board->get_history());

    return string(s_pos);
}