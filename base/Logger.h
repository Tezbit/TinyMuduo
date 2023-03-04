//
// Created by ashoreDove on 2023/2/19.
//
#pragma once
#ifndef TEZBITMUDUO_LOGGER_H
#define TEZBITMUDUO_LOGGER_H

#include "noncopyable.h"

#include <string>

//日志级别：INFO ERROR FATAL DEBUG
enum LogLevel {
    INFO,
    ERROR,
    FATAL, //core
    DEBUG //调试
};

class Logger : noncopyable {
public:
    //懒汉单例
    static Logger &instance();

    //设置日志级别
    void setLogLevel(int level);

    //写日志
    void log(const std::string& msg);

private:
    int logLevel_;

    Logger() = default;
};


#endif //TEZBITMUDUO_LOGGER_H

//LOG_INFO("%s %d",arg1,arg2)
#define LOG_INFO(LogMsgFormat, ...) \
    do \
    { \
        Logger &log=Logger::instance(); \
        log.setLogLevel(INFO);     \
        char buf[1024]={0};        \
        snprintf(buf,1024,LogMsgFormat,##__VA_ARGS__); \
        log.log(buf);\
    }while(0)
#define LOG_ERROR(LogMsgFormat, ...) \
    do \
    { \
        Logger &log=Logger::instance(); \
        log.setLogLevel(ERROR);     \
        char buf[1024]={0};        \
        snprintf(buf,1024,LogMsgFormat,##__VA_ARGS__); \
        log.log(buf);\
    }while(0)
#define LOG_FATAL(LogMsgFormat, ...) \
    do \
    { \
        Logger &log=Logger::instance(); \
        log.setLogLevel(FATAL);     \
        char buf[1024]={0};        \
        snprintf(buf,1024,LogMsgFormat,##__VA_ARGS__); \
        log.log(buf);                \
        exit(-1);                                 \
    }while(0)
#ifdef MUDUO_DEBUG
#define LOG_DEBUG(LogMsgFormat, ...) \
    do \
    { \
        Logger &log=Logger::instance(); \
        log.setLogLevel(DEBUG);     \
        char buf[1024]={0};        \
        snprintf(buf,1024,LogMsgFormat,##__VA_ARGS__); \
        log.log(buf);\
    }while(0)
#else
#define LOG_DEBUG(LogMsgFormat, ...)
#endif
