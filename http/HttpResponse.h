//
// Created by ashoreDove on 2023/3/4.
//
#pragma once
#ifndef TINYMUDUO_HTTPRESPONSE_H
#define TINYMUDUO_HTTPRESPONSE_H


#include <unordered_map>

#include "Buffer.h"

class HttpResponse {
public:
    // 响应状态码
    enum HttpStatusCode {
        hUnknown,
        h200Ok = 200,
        h301MovedPermanently = 301,
        h400BadRequest = 400,
        h404NotFound = 404,
    };
    enum Version {
        kUnknown, kHttp10, kHttp11
    };

    explicit HttpResponse(bool close)
            : statusCode_(hUnknown),
              closeConnection_(close) {
    }

    void setStatusCode(HttpStatusCode code) { statusCode_ = code; }

    void setStatusMessage(const std::string &message) { statusMessage_ = message; }

    void setCloseConnection(bool on) { closeConnection_ = on; }

    bool closeConnection() const { return closeConnection_; }

    void setContentType(const std::string &contentType) { addHeader("Content-Type", contentType); }

    void addHeader(const std::string &key, const std::string &value) { headers_[key] = value; }

    void setBody(const std::string &body) { body_ = body; }

    void appendToBuffer(Buffer *output) const;

private:
    std::unordered_map<std::string, std::string> headers_;
    HttpStatusCode statusCode_;
    Version version_;       // 协议版本号
    std::string statusMessage_;
    bool closeConnection_;
    std::string body_;
};


#endif //TINYMUDUO_HTTPRESPONSE_H
