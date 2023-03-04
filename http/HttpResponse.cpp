//
// Created by ashoreDove on 2023/3/4.
//

#include "HttpResponse.h"

#include <cstdio>
#include <cstring>

void HttpResponse::appendToBuffer(Buffer *output) const {
    // 响应行
    char buf[32];
    memset(buf, '\0', sizeof(buf));
    snprintf(buf, sizeof(buf), "HTTP/1.1 %d ", statusCode_);
    output->append(buf, strlen(buf));
    output->append(statusMessage_.data(), statusMessage_.size());
    output->append("\r\n", 2);
    std::string conn;
    if (closeConnection_) {
        conn = "Connection: close\r\n";
    } else {
        snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", body_.size());
        output->append(buf, strlen(buf));
        conn = "Connection: Keep-Alive\r\n";
    }
    output->append(conn.data(), conn.size());
    for (const auto &header: headers_) {
        output->append(header.first.data(), header.first.size());
        output->append(": ", 2);
        output->append(header.second.data(), header.second.size());
        output->append("\r\n", 2);
    }
    output->append("\r\n", 2);
    output->append(body_.data(), body_.size());
}
