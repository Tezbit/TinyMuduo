//
// Created by ashoreDove on 2023/2/19.
//
#pragma once
#ifndef TEZBITMUDUO_NONCOPYABLE_H
#define TEZBITMUDUO_NONCOPYABLE_H
/*
 * noncopyable 禁止拷贝构造
 * 
 * 
 */

class noncopyable {
public:
    noncopyable(const noncopyable &) = delete;

    void operator=(const noncopyable &) = delete;

protected:
    noncopyable() = default;

    ~noncopyable() = default;
};

#endif //TEZBITMUDUO_NONCOPYABLE_H
