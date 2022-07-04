//
// Created by Kaihua Li on 2022/7/4.
//
#include "../include/Config.h"
#include <regex>

using std::string;
using std::vector;
using std::regex;
using std::sregex_token_iterator;

const string Config::SECTION_MODEL("model");
const string Config::FIELD_MODEL_FILE("file");
const string Config::FIELD_MODEL_LOCAL_GPUIDs("gpuids");
const string Config::FIELD_MODEL_IS_SYNC("is_sync");
const string Config::FIELD_MODEL_AUTHOR("author");
const string Config::FIELD_MODEL_USE_FP16("use_fp16");

const string Config::SECTION_MCTS("mcts");
const string Config::FIELD_MCTS_VIRTUAL_LOSS("virtual_loss");
const string Config::FIELD_MCTS_INIT_TEMPERATURE("init_temperature");
const string Config::FIELD_MCTS_TEMPERATURE_STEP("temperature_step");
const string Config::FIELD_MCTS_C_PUCT("c_puct");
const string Config::FIELD_MCTS_MIN_PROB("min_prob");
const string Config::FIELD_MCTS_N_THREAD("n_thread");
const string Config::FIELD_MCTS_SURRENDER_RATIO("surrender_ratio");
const string Config::FIELD_MCTS_PASS_RATIO("pass_ratio");
const string Config::FIELD_MCTS_PASS_MOVE_NUM("pass_move_num");

const string Config::SECTION_GTP("gtp");
const string Config::FIELD_GTP_NAME("name");
const string Config::FIELD_GTP_VERSION("version");

const string Config::SECTION_TIME_POLICY("time_policy");
const string Config::FIELD_TIME_POLICY_PONDERING("pondering");
const string Config::FIELD_TIME_POLICY_ENABLE("time_policy");
const string Config::FIELD_TIME_POLICY_TIME_OUT_MS("time_out_ms");
const string Config::FIELD_TIME_POLICY_IS_UEC("is_uec");
const string Config::FIELD_TIME_POLICY_IS_CGOS("is_cgos");
const string Config::FIELD_TIME_POLICY_STAGE_0_SAFE_TIME_S("stage_0_safe_time_s");
const string Config::FIELD_TIME_POLICY_STAGE_150_SAFE_TIME_S("stage_150_safe_time_s");
const string Config::FIELD_TIME_POLICY_QUICK_PLAY_MOVE_NUM("quick_play_move_num");
const string Config::FIELD_TIME_POLICY_QUICK_PLAY_MOVE_TIME_OUT_MS("quick_play_move_time_out_ms");
const string Config::FIELD_TIME_POLICY_OVER_USING_THR_TIME_OUT_MS("over_using_thr_time_out_ms");
const string Config::FIELD_TIME_POLICY_UNDER_USING_THR_TIME_OUT_MS("under_using_thr_time_out_ms");
const string Config::FIELD_TIME_POLICY_UNDER_USING_TIME_OUT_MS("under_using_time_out_ms");

const string Config::SECTION_SELFPLAY("selfplay");
const string Config::FIELD_SELFPLAY("selfplay");
const string Config::FIELD_SELFPLAY_EVAL_ROLLOUT("selfplay_eval_rollout");
const string Config::FIELD_SELFPLAY_MIN_MOVE("selfplay_min_move");

const std::string Config::SECTION_EVALUATOR("evaluator");
const std::string Config::FIELD_BATCH_SIZE("batch_size");
const std::string Config::FIELD_SLEEP_THRESHOLD("sleep_threshold");
const std::string Config::FIELD_SLEEP_INTERVAL("sleep_interval");
const std::string Config::FIELD_ICE_CONF("ice_conf");
const std::string Config::FIELD_QUEUE_SIZE("queue_size");
const std::string Config::FIELD_USE_SERVER_EVALUATOR("use_server_evaluator");
const std::string Config::FIELD_SERVER_SEND_THREAD("server_send_thread");
const std::string Config::FIELD_USE_ASYN_REQUEST("use_asyn_request");
const std::string Config::FIELD_ASYN_ONCE_SEND("asyn_once_send");
const std::string Config::FIELD_PROXY_NAME("proxy_name");
const std::string Config::FIELD_PROXY_NUMBER("proxy_number");


// model
string Config::model_file_;
vector<string> Config::model_local_gpuids_;
bool Config::model_is_sync_;
string Config::model_author_;
bool Config::model_use_fp16_;

// mcts
int Config::virtual_loss_;
float Config::init_temperature_;
float Config::temperature_step_;
float Config::c_puct_;
float Config::min_prob_;
int Config::n_thread_;
float Config::resign_ratio_;
float Config::pass_ratio_;
int Config::pass_move_num_;

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
bool Config::use_server_evaluator_;
int Config::server_send_thread_;
bool Config::use_asyn_request_;
int Config::asyn_once_send_;
std::string Config::proxy_name_;
int Config::proxy_number_;

void Config::Sinit(const string& conf_file_path) {
    INIReader reader(conf_file_path);
    if (reader.ParseError() < 0) {
        throw new std::runtime_error("Fail to load configuration file:" + conf_file_path);
    }

    // model
    model_file_ = reader.Get(SECTION_MODEL, FIELD_MODEL_FILE, "");
    string gpuIDstr = reader.Get(SECTION_MODEL, FIELD_MODEL_LOCAL_GPUIDs, "");
    regex regexp{R"(,)"};
    sregex_token_iterator it{gpuIDstr.begin(), gpuIDstr.end(), regexp, -1};
    vector<string> gpuIDs{it, {}};
    model_local_gpuids_ = gpuIDs;
    model_is_sync_ = reader.GetBoolean(SECTION_MODEL, FIELD_MODEL_IS_SYNC, true);
    model_author_  = reader.Get(SECTION_MODEL, FIELD_MODEL_AUTHOR, "");
    model_use_fp16_ = reader.GetBoolean(SECTION_MODEL, FIELD_MODEL_USE_FP16, false);

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
    use_server_evaluator_ = reader.GetBoolean(SECTION_EVALUATOR, FIELD_USE_SERVER_EVALUATOR, true);
    server_send_thread_ = static_cast<int>(reader.GetInteger(SECTION_EVALUATOR, FIELD_SERVER_SEND_THREAD, 1));
    use_asyn_request_ = reader.GetBoolean(SECTION_EVALUATOR, FIELD_USE_ASYN_REQUEST, true);
    asyn_once_send_ = static_cast<int>(reader.GetInteger(SECTION_EVALUATOR, FIELD_ASYN_ONCE_SEND, 1));
    proxy_name_ = reader.Get(SECTION_EVALUATOR, FIELD_PROXY_NAME, "Test");
    proxy_number_ = static_cast<int>(reader.GetInteger(SECTION_EVALUATOR, FIELD_PROXY_NUMBER, 1));
}