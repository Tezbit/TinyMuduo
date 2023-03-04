//
// Created by ashoreDove on 2023/2/23.
//
#pragma once
#ifndef TEZBITMUDUO_BUFFER_H
#define TEZBITMUDUO_BUFFER_H

#include "noncopyable.h"

#include <vector>
#include <string>
#include <algorithm>

/*
 * ----pre----|---------read----------|--------write---------|
 */
class Buffer : noncopyable {
public:
    static const size_t bCheapPrepend = 8;
    static const size_t bInitialSize = 1024;

    explicit Buffer(size_t initialSize = bInitialSize) : buffer_(bCheapPrepend + initialSize),
                                                         readIndex_(bCheapPrepend), writeIndex_(bCheapPrepend) {}

    [[nodiscard]] size_t readableBytes() const { return writeIndex_ - readIndex_; }

    [[nodiscard]] size_t writableBytes() const { return buffer_.size() - writeIndex_; }

    [[nodiscard]] size_t prependableBytes() const { return readIndex_; }

    //返回缓冲区中可读数据的起始地址(指针层面)
    [[nodiscard]] const char *peek() const { return begin() + readIndex_; }

    void retrieve(size_t len) {
        if (len < readableBytes()) {
            readIndex_ += len; //读了一部分，index后移
        } else { //len==readableBytes()
            retrieveAll();
        }
    }

    [[nodiscard]] const char *findCRLF() const {
        const char *crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? nullptr : crlf;
    }

    void retrieveUntil(const char *end) { retrieve(end - peek()); }

    void retrieveAll() {
        readIndex_ = writeIndex_ = bCheapPrepend;
    }

    //onMessage函数上报的Buffer转成string
    std::string retrieveAllAsString() { return retrieveAsString(readableBytes()); }

    std::string retrieveAsString(size_t len) {
        std::string result(peek(), len);
        retrieve(len); //读走了可读区的数据则相关index需要发生改变，进行复位操作
        return result;
    }

    //是否能写入
    void ensureWriteableBytes(size_t len) {
        if (writableBytes() < len) {
            //扩容
            makeSpace(len);
        }
    }

    char *beginWrite() {
        return begin() + writeIndex_;
    }

    [[nodiscard]] const char *beginWrite() const {
        return begin() + writeIndex_;
    }

    //写入
    void append(const char *data, size_t len) {
        ensureWriteableBytes(len);
        std::copy(data, data + len, beginWrite());
        writeIndex_ += len;
    }

    //从fd上读取数据
    ssize_t readFd(int fd, int *savedErrno);

    ssize_t writeFd(int fd, int *savedErrno);

private:
    void makeSpace(size_t len) {
        //可写区域+空闲的大小 < 需要写的大小len+8字节预处理区
        if (writableBytes() + prependableBytes() < len + bCheapPrepend) {
            buffer_.resize(writeIndex_ + len);
        } else {
            //重新调整Index，将空闲区域分配给可写区
            size_t readable = readableBytes();
            std::copy(begin() + readIndex_, begin() + writeIndex_, begin() + bCheapPrepend);
            readIndex_ = bCheapPrepend;
            writeIndex_ = readIndex_ + readable;
        }
    }

    //&(*buffer_.begin())
    char *begin() { return &*buffer_.begin(); }

    [[nodiscard]] const char *begin() const { return &*buffer_.begin(); }

    std::vector<char> buffer_;
    size_t readIndex_;
    size_t writeIndex_;
    static const char kCRLF[];
};


#endif //TEZBITMUDUO_BUFFER_H
