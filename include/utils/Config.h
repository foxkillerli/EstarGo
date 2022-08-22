//
// Created by Kaihua Li on 2022/7/4.
//

#ifndef ESTAR_GO_CONFIG_H
#define ESTAR_GO_CONFIG_H
#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>

class Config {
public:
    static void Sinit(const std::string& conf_file_path);

    static std::string OutputConfig() {
        std::stringstream sstream;
        sstream << "model_type_=" << model_type_ << std::endl
                << "virtual_loss_=" << virtual_loss_ << std::endl
                << "c_puct_=" << c_puct_ << std::endl
                << "policy_prob_threshold_=" << policy_prob_threshold_ << std::endl
                << "search_threads_=" << search_threads_ << std::endl
                << "resign_threshold_" << resign_threshold_ << std::endl
                << "pass_threshold_" << pass_threshold_ << std::endl
                << "pass_after_n_moves_" << pass_after_n_moves_ << std::endl
                << "is_pondering_=" << is_pondering_ << std::endl
                << "is_time_policy_=" << is_time_policy_ << std::endl
                << "ice_conf_=" << ice_conf_ << std::endl
                << "queue_size_=" << queue_size_ << std::endl
                << "use_server_evaluator_=" << use_server_evaluator_ << std::endl
                << "server_send_thread=" << server_send_thread_ << std::endl
                << "use_asyn_request=" << use_asyn_request_ << std::endl
                << "asyn_once_send=" << asyn_once_send_ << std::endl
                << "proxy_name=" << proxy_name_ << std::endl
                << "proxy_number=" << proxy_number_ << std::endl;
        return sstream.str();
    }

public:
    static std::string model_type() { return model_type_; }

    static int virtual_loss() { return virtual_loss_; }
    static float c_puct() { return c_puct_; }
    static float policy_prob_threshold() { return policy_prob_threshold_; }
    static int search_threads() { return search_threads_; }
    static float resign_threshold() {return resign_threshold_; };
    static float pass_threshold() {return pass_threshold_; };
    static int pass_after_n_moves() {return pass_after_n_moves_; };

    static std::string gtp_name() { return gtp_name_; }
    static std::string gtp_version() { return gtp_version_; }

    static bool is_pondering() { return is_pondering_; }
    static bool is_time_policy() { return is_time_policy_; }
    static int time_out_ms() { return time_out_ms_; }
    static int stage_0_safe_time_s() { return stage_0_safe_time_s_; }
    static int stage_150_safe_time_s() { return stage_150_safe_time_s_; }
    static int quick_play_move_num() { return quick_play_move_num_; }
    static int quick_play_move_time_out_ms() { return quick_play_move_time_out_ms_; }
    static int over_using_thr_time_out_ms() { return over_using_thr_time_out_ms_; }
    static int under_using_thr_time_out_ms() { return under_using_thr_time_out_ms_; }
    static int under_using_time_out_ms() { return under_using_time_out_ms_; }


    static int batch_size() { return batch_size_; }
    static int sleep_threshold() { return sleep_threshold_; }
    static int sleep_interval() { return sleep_interval_; }
    static std::string ice_conf() { return ice_conf_; }
    static int queue_size() { return queue_size_; }
    static bool use_server_evaluator() { return use_server_evaluator_; }
    static int server_send_thread() { return server_send_thread_; }
    static bool use_asyn_request() { return use_asyn_request_; }
    static int asyn_once_send() { return asyn_once_send_; }
    static std::string proxy_name() { return proxy_name_; }
    static int proxy_number() { return proxy_number_; }

private:
    Config() {}

private:
    static const std::string SECTION_MODEL;
    static const std::string FIELD_MODEL_TYPE;

    static const std::string SECTION_GTP;
    static const std::string FIELD_GTP_NAME;
    static const std::string FIELD_GTP_VERSION;

    static const std::string SECTION_MCTS;
    static const std::string FIELD_MCTS_VIRTUAL_LOSS;
    static const std::string FIELD_MCTS_C_PUCT;
    static const std::string FIELD_MCTS_POLICY_PROB_THRESHOLD;
    static const std::string FIELD_MCTS_SEARCH_THREADS;
    static const std::string FIELD_MCTS_RESIGN_THRESHOLD;
    static const std::string FIELD_MCTS_PASS_THRESHOLD;
    static const std::string FIELD_MCTS_PASS_AFTER_N_MOVES;


    static const std::string SECTION_TIME_POLICY;
    static const std::string FIELD_TIME_POLICY_PONDERING;
    static const std::string FIELD_TIME_POLICY_ENABLE;
    static const std::string FIELD_TIME_POLICY_TIME_OUT_MS;
    static const std::string FIELD_TIME_POLICY_IS_UEC;
    static const std::string FIELD_TIME_POLICY_IS_CGOS;
    static const std::string FIELD_TIME_POLICY_STAGE_0_SAFE_TIME_S;
    static const std::string FIELD_TIME_POLICY_STAGE_150_SAFE_TIME_S;
    static const std::string FIELD_TIME_POLICY_QUICK_PLAY_MOVE_NUM;
    static const std::string FIELD_TIME_POLICY_QUICK_PLAY_MOVE_TIME_OUT_MS;
    static const std::string FIELD_TIME_POLICY_OVER_USING_THR_TIME_OUT_MS;
    static const std::string FIELD_TIME_POLICY_UNDER_USING_THR_TIME_OUT_MS;
    static const std::string FIELD_TIME_POLICY_UNDER_USING_TIME_OUT_MS;

    static const std::string SECTION_REMOTE;
    static const std::string FIELD_BATCH_SIZE;
    static const std::string FIELD_SLEEP_THRESHOLD;
    static const std::string FIELD_SLEEP_INTERVAL;
    static const std::string FIELD_ICE_CONF;
    static const std::string FIELD_QUEUE_SIZE;
    static const std::string FIELD_USE_SERVER_EVALUATOR;
    static const std::string FIELD_SERVER_SEND_THREAD;
    static const std::string FIELD_USE_ASYN_REQUEST;
    static const std::string FIELD_ASYN_ONCE_SEND;
    static const std::string FIELD_PROXY_NAME;
    static const std::string FIELD_PROXY_NUMBER;

private:
    // model
    static std::string model_type_;

    // mcts
    static int virtual_loss_;
    static float c_puct_;
    static float policy_prob_threshold_;
    static int search_threads_;
    static float resign_threshold_;
    static float pass_threshold_;
    static int pass_after_n_moves_;

    // gtp
    static std::string gtp_name_;
    static std::string gtp_version_;

    // time_policy
    static bool is_pondering_;
    static bool is_time_policy_;
    static int time_out_ms_;
    static int stage_0_safe_time_s_;
    static int stage_150_safe_time_s_;
    static int quick_play_move_num_;
    static int quick_play_move_time_out_ms_;
    static int over_using_thr_time_out_ms_;
    static int under_using_thr_time_out_ms_;
    static int under_using_time_out_ms_;


    //evaluator
    static int batch_size_;
    static int sleep_threshold_;
    static int sleep_interval_;
    static std::string ice_conf_;
    static int queue_size_;
    static bool use_server_evaluator_;
    static int server_send_thread_;
    static bool use_asyn_request_;
    static int asyn_once_send_;
    static std::string proxy_name_;
    static int proxy_number_;
};
#endif //ESTAR_GO_CONFIG_H
