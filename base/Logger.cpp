//
// Created by ashoreDove on 2023/2/19.
//
#include <iostream>

#include "Logger.h"
#include "Timestamp.h"

Logger &Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::setLogLevel(int level) {
    logLevel_ = level;
}

//写日志 [级别] time : msg
void Logger::log(const std::string& msg) {
    switch (logLevel_) {
        case INFO:
            std::cout << "[INFO]";
            break;
        case ERROR:
            std::cout << "[ERROR]";
            break;
        case FATAL:
            std::cout << "[FATAL]";
            break;
        case DEBUG:
            std::cout << "[DEBUG]";
            break;
        default:
            break;
    }
    std::cout << Timestamp::now().toString() << " : " << msg << std::endl;
}
