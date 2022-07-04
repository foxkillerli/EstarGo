//
// Created by Kaihua Li on 2022/7/4.
//

#ifndef ESTAR_GO_LOGGER_H
#define ESTAR_GO_LOGGER_H

#pragma once

#include <string>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

using namespace log4cplus;
using namespace log4cplus::helpers;
// global object
extern Logger logger;
// define some macros for simplicity
#define LOG_ALYS_INFO(logEvent)     LOG4CPLUS_TRACE(logger, logEvent)
#define LOG_DEBUG(logEvent)         LOG4CPLUS_DEBUG(logger, logEvent)
#define LOG_INFO(logEvent)          LOG4CPLUS_INFO(logger, logEvent)
#define LOG_WARN(logEvent)          LOG4CPLUS_WARN(logger, logEvent)
#define LOG_ERROR(logEvent)         LOG4CPLUS_ERROR(logger, logEvent)
#define LOG_FATAL(logEvent)         LOG4CPLUS_FATAL(logger, logEvent)

// global debug file key
extern void InitLogger(const std::string& confFilePath, const std::string& logFilePath);

//extern void ResetLogFile(const std::string& confFileTemplatePath, const std::string& logFilePath);
extern void ResetLogFile(const std::string& confFileTemplatePath);



#endif //ESTAR_GO_LOGGER_H
