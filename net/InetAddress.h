//
// Created by ashoreDove on 2023/2/19.
//
#pragma once
#ifndef TEZBITMUDUO_INETADDRESS_H
#define TEZBITMUDUO_INETADDRESS_H


#include <arpa/inet.h>
#include <string>

class InetAddress {
public:
    explicit InetAddress(uint16_t port, const std::string &ip = "127.0.0.1");

    explicit InetAddress(const sockaddr_in &addr) : addr_(addr) {}

    [[nodiscard]] std::string toIp() const;

    [[nodiscard]] std::string toIpPort() const;

    [[nodiscard]] uint16_t toPort() const;

    [[nodiscard]] const sockaddr_in *getSockAddr() const { return &addr_; };

    void setSockAddr(const sockaddr_in &addr) { addr_ = addr; }

private:
    sockaddr_in addr_;
};


#endif //TEZBITMUDUO_INETADDRESS_H
