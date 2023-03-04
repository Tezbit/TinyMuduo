//
// Created by ashoreDove on 2023/3/4.
//
#pragma once
#ifndef TINYMUDUO_HTTPSERVER_H
#define TINYMUDUO_HTTPSERVER_H

#include "noncopyable.h"
#include "TcpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpContext.h"

class HttpServer : noncopyable {
public:
    using HttpCallback = std::function<void(const HttpRequest &, HttpResponse *)>;

    HttpServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const std::string &name,
               TcpServer::Option option = TcpServer::noReusePort);


    void setHttpCallback(const HttpCallback &cb) {
        httpCallback_ = cb;
    }

    void start();

private:
    void onConnection(const TcpConnectionPtr &conn);

    void onMessage(const TcpConnectionPtr &conn,
                   Buffer *buf,
                   Timestamp receiveTime);

    void onRequest(const TcpConnectionPtr &, const HttpRequest &);

    TcpServer server_;
    HttpCallback httpCallback_;
};


#endif //TINYMUDUO_HTTPSERVER_H
