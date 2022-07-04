//
// Created by Kaihua Li on 2022/7/4.
//

#include <log4cplus/logger.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/configurator.h>
#include <log4cplus/hierarchy.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "utils/Logger.h"

using namespace std;

const std::string LOG_FILE_KEY = "log4cplus.appender.DEBUG_INFO_MSGS.File";

Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("logmain"));

void InitLogger(const string& confFilePath, const string& logFilePath) {
    log4cplus::PropertyConfigurator config(confFilePath.c_str());
    log4cplus::helpers::Properties prop = config.getProperties();
    vector<log4cplus::tstring> pNames = prop.propertyNames();
    time_t timel;
    time(&timel);
    char tmpBuf[255];
    strftime(tmpBuf, 255, "%Y-%m-%d_%H:%M:%S", localtime(&timel));
    string date(tmpBuf, strlen(tmpBuf));
    for (size_t i = 0; i < pNames.size(); ++i) {
        if (pNames[i].find(".File") != string::npos)
            //prop.setProperty("log4cplus." + pNames[i], string("/tmp/trgo") + "." + to_string(timel) + ".log");
            prop.setProperty("log4cplus." + pNames[i], logFilePath + "." + date + ".log");
        else
            prop.setProperty("log4cplus." + pNames[i], prop.getProperty(pNames[i]));
    }
    log4cplus::PropertyConfigurator config2(prop);
    config2.configure();
    //PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT(confFilePath.c_str()));
}

//void ResetLogFile(const string& confFileTemplatePath, const string& logFilePath) {
void ResetLogFile(const string& confFileTemplatePath) {
    log4cplus::PropertyConfigurator config(confFileTemplatePath.c_str());
    log4cplus::helpers::Properties prop = config.getProperties();
    vector<log4cplus::tstring> pNames = prop.propertyNames();
    time_t timel;
    time(&timel);
    for (size_t i = 0; i < pNames.size(); ++i) {
        if (pNames[i].find(".File") != string::npos)
            prop.setProperty("log4cplus." + pNames[i], prop.getProperty(pNames[i]) + "." + to_string(timel));
        else
            prop.setProperty("log4cplus." + pNames[i], prop.getProperty(pNames[i]));
    }
    //prop.setProperty(LOG_FILE_KEY, logFilePath.c_str());
    log4cplus::PropertyConfigurator config2(prop);
    config2.configure();
}
