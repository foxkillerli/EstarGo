//
// Created by Kaihua Li on 2022/7/4.
//

#include <cstdio>
#include <cmath>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <algorithm>
#include <map>
#include <sstream>

#include "MCTSTree.h"
#include "../include/utils/Logger.h"
#include "Config.h"
#include <cstring>
#include <string>
#include <cmath>

using namespace std;
using std::stringstream;
using std::endl;


//select path with branch prune
NodePtr mcts_select_path_root(NodePtr root, int vl, float c_puct, int max, GoBoard* state, int rollout_remain, int & select_depth, int & visit_num, int & score_num){
    if (!root->eval_ready || root->is_terminate){
        return root;
    }
    std::atomic_thread_fence(std::memory_order_acquire);
    //long timeA=getutime();
    ChildrenVecPtr nexts = root->nexts;
    if (nexts->size()==0){
        LOG_FATAL("no child" << state->get_history());
        state->print_board();
        return root;
    }

    int total_n=root->total_n;
    NodePtr best = 0;
    int visit_prune_thr=-1;

    if(max==1){
        for (uint i=0;i<nexts->size();i++){
            if(visit_prune_thr < nexts->at(i)->edge->n - rollout_remain){
                visit_prune_thr = nexts->at(i)->edge->n - rollout_remain;
            }
        }
    }

    float best_score = -1e8;
    float c = c_puct * sqrt(total_n + 1);
    float fa_w=root->value_pred;
    float fa_n=1;
    if(root->edge!=NULL){
        fa_w=root->edge->w;
        fa_n=root->edge->n;
        if(fa_n<1){
            fa_w=root->value_pred;
            fa_n=1;
        }
    }

    visit_num+=nexts->size();
    for (uint i = 0; i < nexts->size(); ++i) {
        NodePtr child=nexts->at(i);
        if (child->is_repetition_checked && child->is_repetition) continue; //this node is repetition action
        if(child->is_terminate){
            if(child->result==max){
                //one child is max, cur_node is terminal with result=max, since current player has one winning choice
                root->is_terminate=true;
                root->result=max;
                best=child;
                break;
            }else if (!best){
                best=child;
            }
        }else{
            if(child->edge->n < visit_prune_thr){
                continue;
            }
            score_num++;
            float score = child->edge->score(c, max,fa_w,fa_n);
            if (best_score < score) {
                best_score = score;
                best = child;
            }
            if(child->edge->n == 0) {
                break;
            }
        }
    }
    //LOG_INFO("size of score num:" << score_num);
    if (best==NULL) return root;
    if(best->is_terminate==true && best->result==-max){
        //all children are -max, cur_node is terminal with result=-max, since current player has no winning choice
        root->is_terminate=true;
        root->result=-max;
    }

    best->edge->vl+=vl;
    //long timeC=getutime();
    //LOG_INFO("cost time of C-B:" << timeC-timeB);

    action_t act = best->edge->action;
    bool succ=state->apply_move(act);
    if(!succ){
        LOG_INFO("[error ] invalidmove " << act);
        state->print_board();
        exit(1);
    }
    //long timeD=getutime();
    //LOG_INFO("cost time of D-C:" << timeD-timeC);
//    assert(succ);
    return mcts_select_path(best, vl, c_puct, -max, state, ++select_depth, visit_num, score_num);
}

#if 0
NodePtr mcts_select_path(NodePtr cur_node, int vl, float c_puct, int max, GoBoard* state, int prune_thr)
{
    //leaf
    if (!cur_node->eval_ready || cur_node->nexts->size()==0 || cur_node->is_terminate){
        return cur_node;
    }

    int total_n=cur_node->total_n;
    NodePtr best = 0;
    //almost to win or lose, prune
    if(total_n>prune_thr && cur_node->edge!=nullptr){
        float wr=(cur_node->edge->w) / (cur_node->edge->n);
        if (wr>0.95 || wr<-0.6){
            cur_node->is_terminate=true;
            cur_node->result=wr>0 ? 1 : -1;
            return cur_node;
        }
    }

    ChildrenVecPtr nexts = cur_node->nexts;
    if(true){
        float best_score = -1e8;
        float c = c_puct * sqrt(total_n + 1);
        float fa_w=cur_node->edge->w;
        float fa_n=cur_node->edge->n;
        for (uint i = 0; i < nexts->size(); ++i) {
            NodePtr child=nexts->at(i);
            if(child->is_terminate){
                if(child->result==max){
                    //one children is max, cur_node is terminal with result=max, since current player has one winning choice
                    cur_node->is_terminate=true;
                    cur_node->result=max;
                    best=child;
                    break;
                }else if (!best){
                    best=child;
                }
            }else{
                float score = child->edge->score(c, max, fa_w, fa_n);
                if (best_score < score) {
                    best_score = score;
                    best = child;
                }
            }
        }
        if(best->is_terminate==true && best->result==-max){
            //all children is -max, cur_node is terminal with result=-max, since current player has no winning choice
            cur_node->is_terminate=true;
            cur_node->result=-max;
        }
    }
    best->edge->vl+=vl;
    action_t act = best->edge->action;
    bool succ=state->apply_move(act);
    if(!succ){
        fprintf(stderr, "[error ] invalidmove %d\n",act);
        state->print_board();
        exit(1);
    }
    assert(succ);
    return mcts_select_path(best, vl, c_puct, -max, state, prune_thr);
}
#endif

NodePtr mcts_select_path(NodePtr cur_node, int vl, float c_puct, int max, GoBoard *state, int & select_depth, int & visit_num, int & score_num){
    if(!cur_node->eval_ready || cur_node->is_terminate){
        return cur_node;
    }
    std::atomic_thread_fence(std::memory_order_acquire);
    ChildrenVecPtr nexts = cur_node->nexts;

    if(nexts->size()==0){
        LOG_FATAL("no child" << state->get_history());
        state->print_board();
        return cur_node;
        //exit(1);
    }

    int total_n=cur_node->total_n;
    NodePtr best = 0;
    float best_score = -1e8;
    float c = c_puct * sqrt(total_n + 1);
    float fa_w=0;
    float fa_n=1;
    if(cur_node->edge!=NULL){
        fa_w=cur_node->edge->w;
        fa_n=cur_node->edge->n;
        if(fa_n<1){
            //child selection before node backup will case edge->n==0, since expantion is done before backup
            //when fa_n , the score function may return wired score thus no best child is selected.
            fa_w=0;
            fa_n=1;
        }
    }

    visit_num += nexts->size();
    for (uint i = 0; i < nexts->size(); ++i) {
        NodePtr child=nexts->at(i);
        if (child->is_repetition_checked && child->is_repetition) continue; //this node is repetition action
        if(child->is_terminate){
            if(child->result==max){
                //one child is max, cur_node is terminal with result=max, since current player has one winning choice
                cur_node->is_terminate=true;
                cur_node->result=max;
                best=child;
                break;
            }else if (!best){
                best=child;
            }
        }else{
            score_num++;
            float score = child->edge->score(c, max, fa_w, fa_n);
            if (best_score < score) {
                best_score = score;
                best = child;
            }
            if(child->edge->n == 0) {
                break;
            }
        }
    }
    if(best->is_terminate==true && best->result==-max){
        //all children are -max, cur_node is terminal with result=-max, since current player has no winning choice
        cur_node->is_terminate=true;
        cur_node->result=-max;
    }

    best->edge->vl+=vl;
    action_t act = best->edge->action;
    bool succ=state->apply_move(act);
    if(!succ){
        LOG_FATAL("invalidmove " << act);
        state->print_board();
        exit(1);
    }
    return mcts_select_path(best, vl, c_puct, -max, state, ++select_depth, visit_num, score_num);
}


/**
 * Phase 4 of MCTS: Backup
 */
void mcts_back_update(NodePtr cur_node, int vl, float z, int invalid_visit)
{
    NodePtr father=cur_node->father.lock();
    while (father!=NULL) {
        int tvl=(invalid_visit+1)*vl;
        cur_node->edge->vl-=tvl;
        cur_node->edge->n++;
        auto current = cur_node->edge->w.load();
        while (!cur_node->edge->w.compare_exchange_weak(current, current + z));
        cur_node = father;
        father = cur_node->father.lock();
//        cur_node = cur_node->father;
        cur_node->total_n++;
    }
}

/**
 * Select the action with max number of visits.
 * action = -1      pass
 * action = -100    resign
 * action = row*19+col
 */
MCTSPair mcts_select_action(NodePtr cur_node, GoBoard *root_board, bool is_opponent_pass)
{
    action_t best_action = -100;
    NodePtr best_node = NULL;
    NodePtr pass_node = NULL;
    int max_visit = -100;
    if(cur_node->nexts ==NULL){
        LOG_FATAL("it should never happend");
        exit(1);
    }
    vector<NodePtr> &nexts = *(cur_node->nexts);

    for (uint i = 0; i < nexts.size(); ++i) {
        if(nexts[i]->edge->action==-1){
            pass_node=nexts[i];
            if (!root_board->is_pass_safe()){
                continue;
            }
            float w = float(pass_node->edge->w);
            int n = float(pass_node->edge->n);
            float win_rate = ((w + n) / 2) / (n + 1e-8);
            if (is_opponent_pass && root_board->get_total_moves() > Config::pass_after_n_moves() && win_rate > Config::pass_threshold()) {
                LOG_INFO("Opponent play pass, and reach pass_move_num and pass_ratio.");
                LOG_INFO("Play responsed pass. ");
                best_action = GoBoard::POS_PASS;
                best_node = pass_node;
                break;
            }
        }
        if(nexts[i]->is_terminate && nexts[i]->result==1){
            best_action = nexts[i]->edge->action;
            best_node = nexts[i];
            break;
        }
        if (max_visit < nexts[i]->edge->n){
            max_visit = nexts[i]->edge->n;
            best_action = nexts[i]->edge->action;
            best_node = nexts[i];
        }
    }

    if(best_action==-100){
        if(pass_node==NULL){
            LOG_FATAL("this should never be happend, no best action");
            exit(1);
        }
        //no sensible move on board, has to pass
        best_action=GoBoard::POS_PASS;
        best_node=pass_node;
    }

    float w=float(best_node->edge->w);
    int n=float(best_node->edge->n);
    float win_rate = ((w+n)/2)/(n+1e-8);

    if(best_action == -1){
        LOG_INFO("best action:pass n:" << n << "w:" << w/n << "win:" << win_rate);
    }else{
        int row=best_action / 19;
        int col=best_action % 19;
        char buf[1024];
        snprintf(buf, 1024, "best action:%c%d n:%d w:%.3f win:%.3f", 'A' + col + (col >= 'I' - 'A'), row + 1, n,w/n,win_rate);
        LOG_INFO(buf);
    }
    if(n>0 && win_rate<Config::resign_threshold()){
        // nearly impossible to win, resign
        LOG_INFO("Nearly to lose, resign");
        best_action = -100;
    }
    //best_node->father = NULL;
    return MCTSPair(best_node, best_action);
}


/**
 * Select the action with distribution before first 30 moves, then with max number of visits.
 * action = -1      pass
 * action = -100    resign
 * action = row*19+col
 */
MCTSPair selfplay_select_action(NodePtr cur_node, GoBoard *root_board, bool is_opponent, default_random_engine &generator)
{
    action_t best_action = -100;
    NodePtr best_node = NULL;
    NodePtr pass_node = NULL;
    int max_visit = -100;
    if(cur_node->nexts ==NULL){
        LOG_FATAL("it should never happend");
        exit(1);
    }
    vector<NodePtr> &nexts = *(cur_node->nexts);

    //compute probability distribution based on visit count
    vector<double> mcts_prob(nexts.size());
    int n_visit_sum = 0;
    stringstream sstream;
    for (uint i = 0; i < nexts.size(); ++i) {
        n_visit_sum += nexts[i]->edge->n;
    }

    sstream << "distribution:";
    std::string policy_distribution = "";
    double policy_dis[362];
    memset(policy_dis, 0, sizeof(policy_dis));
    for (uint i = 0; i < nexts.size(); ++i) {
        mcts_prob[i] = nexts[i]->edge->n * 1.0 / n_visit_sum;
        sstream << nexts[i]->edge->action << ":" << mcts_prob[i];
        int index = nexts[i]->edge->action;
        if (index == -1) index = 361;
        policy_dis[index] = mcts_prob[i];
        if (i == nexts.size() - 1) break;
        sstream << " ";
    }
    for (int i = 0; i < 362; ++i){
        policy_distribution += std::to_string(policy_dis[i]);
        if (i == 361) break;
        policy_distribution += " ";
    }
    LOG_INFO(sstream.str());

    double cross_entropy = 0;
    for (uint i = 0; i < nexts.size(); ++i){
        int index = nexts[i]->edge->action;
        if (index == -1) index = 361;
        if (policy_dis[index] > 0 && nexts[i]->edge->old_p > 0){
            cross_entropy += -policy_dis[index] * log(nexts[i]->edge->old_p);
        }
    }

    if (root_board->get_total_moves() >= 30) {
        for (uint i = 0; i < nexts.size(); ++i) {
            if(nexts[i]->edge->action==-1){
                pass_node=nexts[i];
                if (!root_board->is_pass_safe()){
                    continue;
                }
            }
            if(nexts[i]->is_terminate && nexts[i]->result==1){
                best_action = nexts[i]->edge->action;
                best_node = nexts[i];
                break;
            }
            if (max_visit < nexts[i]->edge->n){
                max_visit = nexts[i]->edge->n;
                best_action = nexts[i]->edge->action;
                best_node = nexts[i];
            }
        }
    } else {
        //using MCTS visit_count distribution for move sampling
        discrete_distribution<int> distribution(mcts_prob.begin(), mcts_prob.end());
        int select = distribution(generator);
        best_action = nexts[select]->edge->action;
        best_node = nexts[select];
    }

    if(best_action==-100){
        if(pass_node==NULL){
            LOG_FATAL("this should never be happend, no best action");
            exit(1);
        }
        //no sensible move on board, has to pass
        best_action=GoBoard::POS_PASS;
        best_node=pass_node;
    }

    float w=float(best_node->edge->w);
    if (is_opponent) w = w * -1;
    int n=float(best_node->edge->n);
    float win_rate = ((w+n)/2)/(n+1e-8);

    if(best_action == -1){
        LOG_INFO("best action:pass n:" << n <<" w:" << w/n << " win:" << win_rate);
    }else{
        int row=best_action / 19;
        int col=best_action % 19;
        char buf[1024];
        snprintf(buf, 1024, "[info ] best action:%c%d n:%d w:%.3f win:%.3f\n", 'A' + col + (col >= 'I' - 'A'), row + 1, n,w/n,win_rate);
        LOG_INFO(buf);
    }
    if (win_rate<Config::resign_threshold()){
        best_action = -100;
    }
    (best_node->father).reset();
    MCTSPair mcts_pair = MCTSPair(best_node, best_action);
    mcts_pair.policy_label = policy_distribution;
    mcts_pair.win_label = w/n;
    mcts_pair.win_rate = win_rate;
    mcts_pair.cross_entropy = cross_entropy;
    return mcts_pair;
}

void dump_best_move(NodePtr cur_node, int depth, int prune_thr) {
    stringstream sstream;
    dump_best_move_internal(cur_node, depth, prune_thr, sstream);
    LOG_INFO("\n" << sstream.str());
}

void dump_best_move_internal(NodePtr cur_node, int depth, int prune_thr, stringstream& sstream)
{
    if(cur_node->nexts == NULL){
        return;
    }
    vector<NodePtr> &nexts = *(cur_node->nexts);

    //sort by prior
    std::map<float, int> point_prob;
    for (uint i = 0; i < nexts.size(); i++) {
        point_prob[nexts[i]->edge->p] = i;
    }

    map<float, int>::reverse_iterator ite;
    for (ite = point_prob.rbegin(); ite != point_prob.rend(); ++ite) {
        int i = ite->second;

        int total_n=cur_node->total_n;
        int n= nexts[i]->edge->n;
        //if(n>prune_thr && (depth==0 || (((0.0+n)/total_n>0.2 && depth<3)|| (0.0+n)/total_n>0.5))){
        if((depth==0 || (((0.0+n)/total_n>0.2 && depth<3)|| (0.0+n)/total_n>0.5))){
            //if(n>0 && depth<3){
            int row=nexts[i]->edge->action/19;
            int col=nexts[i]->edge->action%19;
            int n=nexts[i]->edge->n;
            float w=nexts[i]->edge->w;
            char buf[1024];
            snprintf(buf, 1024, "%d%*s [%c%d n:%d w:%.3f pri:%.3f val:%.3f out:%d %c%c]\n", depth, depth,"",
                     'A' + col + (col >= 'I' - 'A'),
                     row + 1,
                     n,
                     w/n,
                     nexts[i]->edge->p,
                     nexts[i]->value_pred,
                     nexts[i]->result,
                     nexts[i]->is_terminate?'T':' ',
                     nexts[i]->eval_ready?'E':' '
            );
            sstream << buf;
            dump_best_move_internal(nexts[i],depth+1, prune_thr, sstream);
        }
    }
}

void dump_all_move(char *history_buf,int history_len,NodePtr cur_node, int is_black)
{
    if(cur_node->nexts == NULL){
        return;
    }
    stringstream sstream;
    vector<NodePtr> &nexts = *(cur_node->nexts);
    for (uint i = 0; i<nexts.size(); i++){
        if(nexts[i]->edge->n>100){
            int row=nexts[i]->edge->action/19;
            int col=nexts[i]->edge->action%19;
            history_buf[history_len]='A'+col;
            history_buf[history_len+1]='a'+row;
            history_buf[history_len+2]=0;
            float w=nexts[i]->edge->w/nexts[i]->edge->n;
            if(fabs(w-nexts[i]->value_pred)>0.8){
                char buf[1024];
                snprintf(buf, 1024, "[explorer] %8d\t%6d\t%.3f\t%6d\t%.3f\t%.3f\t%.3f\t%s\n", (int)cur_node->total_n,(int)nexts[i]->edge->n, w * is_black, (int)nexts[i]->edge->n, w * is_black, nexts[i]->edge->p,nexts[i]->value_pred*is_black, history_buf);
                sstream << buf << endl;
            }
            LOG_DEBUG(nexts[i]->edge->p << " " << cur_node->value_pred*is_black << " " << history_buf);
            dump_all_move(history_buf, history_len+2, nexts[i], is_black);
        }
    }
    LOG_INFO(sstream.str());
}


void collect_tree_info(NodePtr cur_node, int depth, TreeInfo* info){
    //TODO
    if(cur_node->edge!=NULL && cur_node->edge->vl != 0){
        LOG_INFO("non zero vl");
    }
    info->node_number += 1;
    info->avg_depth += depth;
    if (depth > info->max_depth) info->max_depth = depth;
    info->non_leaf_node_number += (cur_node->edge != NULL && cur_node->edge->n > 1);

    if(cur_node->eval_ready){
        info->collect(depth);
        vector<NodePtr> &nexts = *(cur_node->nexts);
        for (uint i = 0; i<nexts.size(); i++){
            if (nexts[i]->edge != NULL && nexts[i]->edge->n > 0) info->avg_width += 1;

            collect_tree_info(nexts[i],depth+1,info);
        }
    }
}

