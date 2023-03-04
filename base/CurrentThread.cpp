//
// Created by ashoreDove on 2023/2/21.
//


#include "CurrentThread.h"
namespace CurrentThread{
    __thread int t_cachedTid=0;
    void cacheTid(){
        if(t_cachedTid==0){
            //系统调用获取当前线程tid
            t_cachedTid=static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }
}