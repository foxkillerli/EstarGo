//
// Created by Kaihua Li on 2022/7/4.
//

#ifndef ESTAR_GO_CONFIG_H
#define ESTAR_GO_CONFIG_H
#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>
#include "utils/INIReader.h"

class Config {
public:
    static void Sinit(const std::string& conf_file_path);

    static std::string OutputConfig() {
        std::stringstream sstream;
        sstream << "model_file_=" << model_file_ << std::endl
                << "model_local_gpuids_=" << strvec2str(model_local_gpuids_) << std::endl
                << "model_is_sync_=" << model_is_sync_ << std::endl
                << "virtual_loss_=" << virtual_loss_ << std::endl
                << "c_puct_=" << c_puct_ << std::endl
                << "init_temperature_=" << init_temperature_ << std::endl
                << "temperature_step_=" << temperature_step_ << std::endl
                << "min_prob_=" << min_prob_ << std::endl
                << "n_thread_=" << n_thread_ << std::endl
                << "resign_ratio_" << resign_ratio_ << std::endl
                << "pass_ratio_" << pass_ratio_ << std::endl
                << "pass_move_num_" << pass_move_num_ << std::endl
                << "is_pondering_=" << is_pondering_ << std::endl
                << "is_time_policy_=" << is_time_policy_ << std::endl
                << "time_out_ms_=" << time_out_ms_ << std::endl
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
    static std::string model_file() { return model_file_; }
    static std::vector<std::string>& model_local_gpuids() { return model_local_gpuids_; }
    static bool model_is_sync() { return model_is_sync_; }
    static std::string model_author() {return model_author_; }
    static bool model_use_fp16() { return model_use_fp16_; }

    static int virtual_loss() { return virtual_loss_; }
    static float c_puct() { return c_puct_; }
    static float init_temperature() { return init_temperature_; }
    static float temperature_step() { return temperature_step_; }
    static float min_prob() { return min_prob_; }
    static int n_thread() { return n_thread_; }
    static float resign_ratio() {return resign_ratio_; };
    static float pass_ratio() {return pass_ratio_; };
    static int pass_move_num() {return pass_move_num_; };

    static std::string gtp_name() { return gtp_name_; }
    static std::string gtp_version() { return gtp_version_; }

    static bool is_pondering() { return is_pondering_; }
    static bool is_time_policy() { return is_time_policy_; }
    static int time_out_ms() { return time_out_ms_; }
    static bool is_uec() { return is_uec_; }
    static bool is_cgos() { return is_cgos_; }
    static int stage_0_safe_time_s() { return stage_0_safe_time_s_; }
    static int stage_150_safe_time_s() { return stage_150_safe_time_s_; }
    static int quick_play_move_num() { return quick_play_move_num_; }
    static int quick_play_move_time_out_ms() { return quick_play_move_time_out_ms_; }
    static int over_using_thr_time_out_ms() { return over_using_thr_time_out_ms_; }
    static int under_using_thr_time_out_ms() { return under_using_thr_time_out_ms_; }
    static int under_using_time_out_ms() { return under_using_time_out_ms_; }

    static int selfplay() { return selfplay_; }
    static int selfplay_eval_rollout() { return selfplay_eval_rollout_;}
    static int selfplay_min_move() { return selfplay_min_move_; }

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
    static std::string strvec2str(const std::vector<std::string>& vec) {
        std::string ret;
        for (size_t i = 0; i < vec.size(); ++i) {
            ret += vec[i];
        }
        return ret;
    }

private:
    static const std::string SECTION_MODEL;
    static const std::string FIELD_MODEL_FILE;
    static const std::string FIELD_MODEL_LOCAL_GPUIDs;
    static const std::string FIELD_MODEL_IS_SYNC;
    static const std::string FIELD_MODEL_AUTHOR;
    static const std::string FIELD_MODEL_USE_FP16;

    static const std::string SECTION_MCTS;
    static const std::string FIELD_MCTS_VIRTUAL_LOSS;
    static const std::string FIELD_MCTS_C_PUCT;
    static const std::string FIELD_MCTS_INIT_TEMPERATURE;
    static const std::string FIELD_MCTS_TEMPERATURE_STEP;
    static const std::string FIELD_MCTS_MIN_PROB;
    static const std::string FIELD_MCTS_N_THREAD;
    static const std::string FIELD_MCTS_SURRENDER_RATIO;
    static const std::string FIELD_MCTS_PASS_RATIO;
    static const std::string FIELD_MCTS_PASS_MOVE_NUM;

    static const std::string SECTION_GTP;
    static const std::string FIELD_GTP_NAME;
    static const std::string FIELD_GTP_VERSION;

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

    static const std::string SECTION_SELFPLAY;
    static const std::string FIELD_SELFPLAY;
    static const std::string FIELD_SELFPLAY_EVAL_ROLLOUT;
    static const std::string FIELD_SELFPLAY_MIN_MOVE;

    static const std::string SECTION_EVALUATOR;
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
    static std::string model_file_;
    static std::vector<std::string> model_local_gpuids_;
    static bool model_is_sync_;
    static std::string model_author_;
    static bool model_use_fp16_;

    // mcts
    static int virtual_loss_;
    static float init_temperature_;
    static float temperature_step_;
    static float c_puct_;
    static float min_prob_;
    static int n_thread_;
    static float resign_ratio_;
    static float pass_ratio_;
    static int pass_move_num_;

    // gtp
    static std::string gtp_name_;
    static std::string gtp_version_;

    // time_policy
    static bool is_pondering_;
    static bool is_time_policy_;
    static int time_out_ms_;
    static bool is_uec_;
    static bool is_cgos_;
    static int stage_0_safe_time_s_;
    static int stage_150_safe_time_s_;
    static int quick_play_move_num_;
    static int quick_play_move_time_out_ms_;
    static int over_using_thr_time_out_ms_;
    static int under_using_thr_time_out_ms_;
    static int under_using_time_out_ms_;

    //selfplay
    static int selfplay_;
    static int selfplay_eval_rollout_;
    static int selfplay_min_move_;

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
