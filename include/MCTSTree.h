//
// Created by Kaihua Li on 2022/7/4.
//

#ifndef ESTAR_GO_MCTSTREE_H
#define ESTAR_GO_MCTSTREE_H


#include <vector>
#include <memory>
#include <atomic>
#include <sstream>

#include "GoBoard.h"
#include <cstring>
#include <random>

typedef std::shared_ptr<GoBoard> GoBoardPtr;
typedef int action_t;

/**
 * A class reprents edges between nodes in MCTS Tree
 */
struct Edge
{
    action_t action; /**< Position of move */
    float p; /**< Prior prob */
    double old_p;
    std::atomic_int n;
    std::atomic<float> w;
    std::atomic_int vl;
    /**
     * Initialize the Edge
     */
    Edge(action_t action, float p, int child)
    {
        this->action = action;
        this->p = p;
        this->n = 0;
        this->w = 0;
        this->vl = 0;
        this->old_p = p;
    }

    /**
     * Input: c = c_puct * sqrt{\sum_b nr_b}
     * This should be given by caller.
     * c_puct is the exploration constant
     * This calculates the utility of a node using UCT formula.
     */
    inline float score(float c, int max, float fa_w, float fa_n) {
        int b = vl;
        int q = vl * max;
        if(n!=0){
            return c * p / (1 + n + b) + (w - q) / (n + b) * max;
        }else{
            return c * p / (1 + n + b) + (fa_w - q) / (fa_n + b + 0.1f) * max;
        }
    }
};

typedef std::shared_ptr<Edge> EdgePtr;

struct Node;

typedef std::shared_ptr<Node> NodePtr;
typedef std::weak_ptr<Node> FatherPtr;
typedef std::shared_ptr<std::vector<NodePtr> > ChildrenVecPtr;

/**
 * A class reprents nodes in MCTS Tree
 */
struct Node
{
    ChildrenVecPtr nexts; /**< List of Childs */
    FatherPtr father; /**< Node's father */
    EdgePtr edge; /**< edge from father to this node*/

    std::atomic_int total_n; /**< Total number of rollouts. This variable is volatile because it may be modified by different threads. */
    std::atomic_int invalid_visit;

    volatile int depth;
    volatile float value_pred; //value prediction
    volatile bool rq_sent;
    volatile bool eval_ready;
    volatile bool is_terminate;
    bool is_repetition_checked;
    bool is_repetition;
    volatile int result;
    bool add_dirichlet;
    volatile int dirichlet_max_action;

    Node(NodePtr father = 0, EdgePtr edge = 0, int idx = 0)
    {
        this->father = father;
        nexts = 0;
        this->edge = edge;
        total_n = 0;
        value_pred = 0.;
        rq_sent=false;
        eval_ready=false;
        is_terminate = false;
        depth=0;
        result=0;
        invalid_visit=0;
        is_repetition_checked = false;
        is_repetition = false;
        add_dirichlet = false;
        dirichlet_max_action = -1;
    }

    ~Node() {
    }
};

/**
 * A class representing the pair <Node, Action>, which will be returned to the caller of MCTS.genmove()
 */
struct MCTSPair
{
    NodePtr node; /**< The child corresponds to the selected action. This will be our next root of Monte-Carlo Tree. */
    action_t action; /**< The action chosen */
    std::string policy_label;
    float win_label;
    float win_rate;
    float cross_entropy;

    MCTSPair(NodePtr n, action_t act = 0)
    {
        node = n;
        action = act;
        policy_label = "";
        win_label = 0;
        win_rate = 0;
        cross_entropy = 0;
    }
};

class TreeInfo
{
public:
    int width[1000];
    int max_depth=0;
    int avg_depth=0;
    int avg_width=0; //average children number
    int max_width=0;
    int node_number=0;
    int non_leaf_node_number=0;

    TreeInfo(){
        memset(width,0,sizeof(width));
        max_depth=-1;

        avg_depth=0;
        avg_width=0; //average children number
        max_width=-1;
        node_number=0;
        non_leaf_node_number=0;
    }
    void collect(int depth){
        if(depth>max_depth){
            max_depth=depth;
        }
        width[depth]++;
    }
};

/*NodePtr mcts_select_path(NodePtr cur_node, int vl, float c_puct, int max, GoBoard* state);*/
NodePtr mcts_select_path_root(NodePtr root, int vl, float c_puct, int max, GoBoard* state, int rollout_remain, int & select_depth, int & visit_num, int & score_num);
NodePtr mcts_select_path(NodePtr root, int vl, float c_puct, int max, GoBoard* state, int & select_depth, int & visit_num, int & score_num);

void mcts_back_update(NodePtr cur_node, int vl, float zr, int invalid_visit);
void mcts_free(NodePtr node);
MCTSPair mcts_select_action(NodePtr cur_node, GoBoard *root_board, bool is_opponent_pass=false);
MCTSPair selfplay_select_action(NodePtr cur_node, GoBoard *root_board, bool is_opponent, std::default_random_engine &generator);

void selfplay_dump_move(NodePtr cur_node, int depth, bool is_opponent);
void dump_best_move(NodePtr cur_node, int depth, int prune_thr);
void dump_best_move_internal(NodePtr cur_node, int depth, int prune_thr, std::stringstream& sstream);
void dump_all_move(char *history_buf,int history_len,NodePtr cur_node, int is_black);
void collect_tree_info(NodePtr, int depth, TreeInfo *info);


#endif //ESTAR_GO_MCTSTREE_H
