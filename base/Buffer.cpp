//
// Created by ashoreDove on 2023/2/23.
//

#include "Buffer.h"

#include <cerrno>
#include <sys/uio.h>
#include <unistd.h>

const char Buffer::kCRLF[] = "\r\n";

ssize_t Buffer::readFd(int fd, int *savedErrno) {
    char extrabuf[65526] = {0}; //栈上内存空间 64k
    struct iovec vec[2];
    const size_t writable = writableBytes(); //buffer底层缓存区剩余可写部分的大小
    vec[0].iov_base = begin() + writeIndex_;
    vec[0].iov_len = writable;
    //装不下先装到栈上
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    //如果Buffer可写区小于64k则需要vec的两个成员，否则只需要一个
    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0) {
        *savedErrno = errno;
    } else if (n <= writable) {
        //buffer足够写入
        writeIndex_ += n;
    } else {
        //将vec[1]装入buffer
        writeIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    };
    //返回读取字节数
    return n;
}

ssize_t Buffer::writeFd(int fd, int *savedErrno) {
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0) {
        *savedErrno = errno;
    }
    return n;
}


