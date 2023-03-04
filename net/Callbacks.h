//
// Created by ashoreDove on 2023/2/23.
//
#pragma once
#ifndef TEZBITMUDUO_CALLBACKS_H
#define TEZBITMUDUO_CALLBACKS_H

#include <memory>
#include <functional>

class Buffer;

class TcpConnection;

class Timestamp;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
using MessageCallback = std::function<void(const TcpConnectionPtr &, Buffer *, Timestamp)>;


#endif //TEZBITMUDUO_CALLBACKS_H
