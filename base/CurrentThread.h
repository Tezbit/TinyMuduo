//
// Created by ashoreDove on 2023/2/21.
//
#pragma once
#ifndef TEZBITMUDUO_CURRENTTHREAD_H
#define TEZBITMUDUO_CURRENTTHREAD_H

#include <syscall.h>
#include <unistd.h>


namespace CurrentThread {
    extern __thread int t_cachedTid;

    void cacheTid();

    inline int tid() {
        if (__builtin_expect(t_cachedTid == 0, 0)) {
            cacheTid();
        }
        return t_cachedTid;
    }
}

#endif //TEZBITMUDUO_CURRENTTHREAD_H
