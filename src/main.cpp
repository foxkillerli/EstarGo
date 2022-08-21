//
// Created by Kaihua Li on 2022/7/3.
//
#include <iostream>
#include <gflags/gflags.h>
#include "utils/Logger.h"
#include "Gtp.h"
#include "utils/Config.h"

using std::cout;
using std::endl;

DEFINE_string(config, "conf/conf.ini", "config file pathname");
DEFINE_string(log_config, "conf/log.conf", "log config file pathname");
DEFINE_string(log_output, "/tmp/estar_go_log", "log output file pathname");
DEFINE_string(history, "", "history for bad case test, or continue playing");
DEFINE_string(output_file, "", "output file path");
DEFINE_string(win_rate_file, "", "output min win rate file path");

int main(int argc, char *argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    InitLogger(FLAGS_log_config, FLAGS_log_output);

    Config::Sinit(FLAGS_config);
    LOG_INFO(Config::OutputConfig());
    GTP *g = new GTP();
    LOG_INFO("Start initialization.");
    g->initialize(FLAGS_history);
    LOG_INFO("Initialization finished, let's rock.");
    g->run();
    delete g;
}