//
// Created by ashoreDove on 2023/2/19.
//
#pragma once
#ifndef TEZBITMUDUO_TIMESTAMP_H
#define TEZBITMUDUO_TIMESTAMP_H


#include <iostream>
#include <string>

class Timestamp {
public:
    Timestamp();
    //显式构造
    explicit Timestamp(int64_t microSecondsSinceEpoch);

    static Timestamp now();

    //只读
    [[nodiscard]] std::string toString() const;

private:
    int64_t microSecondsSinceEpoch_;
};


#endif //TEZBITMUDUO_TIMESTAMP_H
