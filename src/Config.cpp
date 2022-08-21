//
// Created by Kaihua Li on 2022/7/4.
//
#include "../include/utils/Config.h"
#include "../include/utils/INIReader.h"
#include <regex>

using std::string;
using std::vector;
using std::regex;
using std::sregex_token_iterator;
const string Config::SECTION_GTP("GTP");
const string Config::FIELD_GTP_NAME("name");
const string Config::FIELD_GTP_VERSION("version");

const string Config::SECTION_MODEL("Model");
const string Config::FIELD_MODEL_TYPE("type");

const string Config::SECTION_MCTS("MCTS");
const string Config::FIELD_MCTS_VIRTUAL_LOSS("virtual_loss");
const string Config::FIELD_MCTS_C_PUCT("c_puct");
const string Config::FIELD_MCTS_POLICY_PROB_THRESHOLD("policy_prob_threshold");
const string Config::FIELD_MCTS_SEARCH_THREADS("search_threads");
const string Config::FIELD_MCTS_RESIGN_THRESHOLD("resign_threshold");
const string Config::FIELD_MCTS_PASS_THRESHOLD("pass_threshold");
const string Config::FIELD_MCTS_PASS_AFTER_N_MOVES("pass_after_n_moves");

const string Config::SECTION_TIME_POLICY("TimePolicy");
const string Config::FIELD_TIME_POLICY_PONDERING("pondering");
const string Config::FIELD_TIME_POLICY_ENABLE("time_policy_enabled");
const string Config::FIELD_TIME_POLICY_TIME_OUT_MS("time_out_ms");
const string Config::FIELD_TIME_POLICY_STAGE_0_SAFE_TIME_S("stage_0_safe_time_s");
const string Config::FIELD_TIME_POLICY_STAGE_150_SAFE_TIME_S("stage_150_safe_time_s");
const string Config::FIELD_TIME_POLICY_QUICK_PLAY_MOVE_NUM("quick_play_move_num");
const string Config::FIELD_TIME_POLICY_QUICK_PLAY_MOVE_TIME_OUT_MS("quick_play_move_time_out_ms");
const string Config::FIELD_TIME_POLICY_OVER_USING_THR_TIME_OUT_MS("over_using_thr_time_out_ms");
const string Config::FIELD_TIME_POLICY_UNDER_USING_THR_TIME_OUT_MS("under_using_thr_time_out_ms");
const string Config::FIELD_TIME_POLICY_UNDER_USING_TIME_OUT_MS("under_using_time_out_ms");

const std::string Config::SECTION_REMOTE("Remote");
const std::string Config::FIELD_BATCH_SIZE("batch_size");
const std::string Config::FIELD_SLEEP_THRESHOLD("sleep_threshold");
const std::string Config::FIELD_SLEEP_INTERVAL("sleep_interval");
const std::string Config::FIELD_ICE_CONF("ice_conf");
const std::string Config::FIELD_QUEUE_SIZE("queue_size");
const std::string Config::FIELD_SERVER_SEND_THREAD("server_send_thread");
const std::string Config::FIELD_USE_ASYN_REQUEST("use_asyn_request");
const std::string Config::FIELD_ASYN_ONCE_SEND("asyn_once_send");
const std::string Config::FIELD_PROXY_NAME("proxy_name");
const std::string Config::FIELD_PROXY_NUMBER("proxy_number");


// model
string Config::model_type_;

// mcts
int Config::virtual_loss_;
float Config::c_puct_;
float Config::policy_prob_threshold_;
int Config::search_threads_;
float Config::resign_threshold_;
float Config::pass_threshold_;
int Config::pass_after_n_moves_;

// gtp
string Config::gtp_name_;
string Config::gtp_version_;

// time_policy
bool Config::is_pondering_;
bool Config::is_time_policy_;
int Config::time_out_ms_;
bool Config::is_uec_;
bool Config::is_cgos_;
int Config::stage_0_safe_time_s_;
int Config::stage_150_safe_time_s_;
int Config::quick_play_move_num_;
int Config::quick_play_move_time_out_ms_;
int Config::over_using_thr_time_out_ms_;
int Config::under_using_thr_time_out_ms_;
int Config::under_using_time_out_ms_;

//selfplay
int Config::selfplay_;
int Config::selfplay_eval_rollout_;
int Config::selfplay_min_move_;

//evaluator
int Config::batch_size_;
int Config::sleep_threshold_;
int Config::sleep_interval_;
std::string Config::ice_conf_;
int Config::queue_size_;
int Config::server_send_thread_;
bool Config::use_asyn_request_;
int Config::asyn_once_send_;
std::string Config::proxy_name_;
int Config::proxy_number_;

void Config::Sinit(const string& conf_file_path) {
    IniReader reader;
    if (!reader.OpenFile(conf_file_path.c_str())) {
        std::cerr << "open config file failed" << std::endl;
        exit(1);
    }

    // model
    model_type_ = reader.GetString(SECTION_MODEL, FIELD_MODEL_TYPE);

    // mcts
    virtual_loss_ = static_cast<int>(reader.GetInteger(SECTION_MCTS, FIELD_MCTS_VIRTUAL_LOSS, 3));
    c_puct_ = static_cast<float>(reader.GetReal(SECTION_MCTS, FIELD_MCTS_C_PUCT, 1));
    init_temperature_ = static_cast<float>(reader.GetReal(SECTION_MCTS, FIELD_MCTS_INIT_TEMPERATURE, 1.0));
    temperature_step_ = static_cast<float>(reader.GetReal(SECTION_MCTS, FIELD_MCTS_TEMPERATURE_STEP, 0.0));
    min_prob_ = static_cast<float>(reader.GetReal(SECTION_MCTS, FIELD_MCTS_MIN_PROB, 0.001));
    n_thread_ = static_cast<int>(reader.GetInteger(SECTION_MCTS, FIELD_MCTS_N_THREAD, 1));
    resign_ratio_ = static_cast<float>(reader.GetReal(SECTION_MCTS, FIELD_MCTS_SURRENDER_RATIO, 0.1));
    pass_ratio_ = static_cast<float>(reader.GetReal(SECTION_MCTS, FIELD_MCTS_PASS_RATIO, 0.9));
    pass_move_num_ = static_cast<int>(reader.GetInteger(SECTION_MCTS, FIELD_MCTS_PASS_MOVE_NUM, 200));

    // gtp
    gtp_name_ = reader.Get(SECTION_GTP, FIELD_GTP_NAME, "ybot");
    gtp_version_ = reader.Get(SECTION_GTP, FIELD_GTP_VERSION, "1.0");

    // time_policy
    is_pondering_ = reader.GetBoolean(SECTION_TIME_POLICY, FIELD_TIME_POLICY_PONDERING, true);
    is_time_policy_ = reader.GetBoolean(SECTION_TIME_POLICY, FIELD_TIME_POLICY_ENABLE, true);
    time_out_ms_ = static_cast<int>(reader.GetInteger(SECTION_TIME_POLICY, FIELD_TIME_POLICY_TIME_OUT_MS, 15000));
    is_uec_ = reader.GetBoolean(SECTION_TIME_POLICY, FIELD_TIME_POLICY_IS_UEC, false);
    is_cgos_ = reader.GetBoolean(SECTION_TIME_POLICY, FIELD_TIME_POLICY_IS_CGOS, true);
    stage_0_safe_time_s_ = static_cast<int>(reader.GetInteger(SECTION_TIME_POLICY, FIELD_TIME_POLICY_STAGE_0_SAFE_TIME_S, 100));
    stage_150_safe_time_s_ = static_cast<int>(reader.GetInteger(SECTION_TIME_POLICY, FIELD_TIME_POLICY_STAGE_150_SAFE_TIME_S, 30));
    quick_play_move_num_ = static_cast<int>(reader.GetInteger(SECTION_TIME_POLICY, FIELD_TIME_POLICY_QUICK_PLAY_MOVE_NUM, 3));
    quick_play_move_time_out_ms_ = static_cast<int>(reader.GetInteger(SECTION_TIME_POLICY, FIELD_TIME_POLICY_QUICK_PLAY_MOVE_TIME_OUT_MS, 1000));
    over_using_thr_time_out_ms_ = static_cast<int>(reader.GetInteger(SECTION_TIME_POLICY, FIELD_TIME_POLICY_OVER_USING_THR_TIME_OUT_MS, 3000));
    under_using_thr_time_out_ms_ = static_cast<int>(reader.GetInteger(SECTION_TIME_POLICY, FIELD_TIME_POLICY_UNDER_USING_THR_TIME_OUT_MS, 2000));
    under_using_time_out_ms_ = static_cast<int>(reader.GetInteger(SECTION_TIME_POLICY, FIELD_TIME_POLICY_UNDER_USING_TIME_OUT_MS, 1000));

    // selfplay
    selfplay_ = static_cast<int>(reader.GetInteger(SECTION_SELFPLAY, FIELD_SELFPLAY, 0));
    selfplay_eval_rollout_ = static_cast<int>(reader.GetInteger(SECTION_SELFPLAY, FIELD_SELFPLAY_EVAL_ROLLOUT, 0));
    selfplay_min_move_ = static_cast<int>(reader.GetInteger(SECTION_SELFPLAY, FIELD_SELFPLAY_MIN_MOVE, 92));

    // evaluator
    batch_size_ = static_cast<int>(reader.GetInteger(SECTION_EVALUATOR, FIELD_BATCH_SIZE, 4));
    sleep_threshold_ = static_cast<int>(reader.GetInteger(SECTION_EVALUATOR, FIELD_SLEEP_THRESHOLD, 10));
    sleep_interval_ = static_cast<int>(reader.GetInteger(SECTION_EVALUATOR, FIELD_SLEEP_INTERVAL, 1000));
    ice_conf_ = reader.Get(SECTION_EVALUATOR, FIELD_ICE_CONF, "");
    queue_size_ = static_cast<int>(reader.GetInteger(SECTION_EVALUATOR, FIELD_QUEUE_SIZE, 1000));
    server_send_thread_ = static_cast<int>(reader.GetInteger(SECTION_EVALUATOR, FIELD_SERVER_SEND_THREAD, 1));
    use_asyn_request_ = reader.GetBoolean(SECTION_EVALUATOR, FIELD_USE_ASYN_REQUEST, true);
    asyn_once_send_ = static_cast<int>(reader.GetInteger(SECTION_EVALUATOR, FIELD_ASYN_ONCE_SEND, 1));
    proxy_name_ = reader.Get(SECTION_EVALUATOR, FIELD_PROXY_NAME, "Test");
    proxy_number_ = static_cast<int>(reader.GetInteger(SECTION_EVALUATOR, FIELD_PROXY_NUMBER, 1));
}