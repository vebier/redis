#include "network/connection.h"
#include <unistd.h>

Connection::Connection(int fd) : fd_(fd) {
    want_read = true;
}

Connection::~Connection() {
    if (fd_ >= 0) {
        close(fd_);
    }
}

Connection::Connection(Connection&& other) noexcept
        : fd_(other.fd_)
        , want_read(other.want_read)
        , want_write(other.want_write)
        , want_close(other.want_close)
        , incoming(std::move(other.incoming))
        , outgoing(std::move(other.outgoing)) {
    other.fd_ = -1;
}

int Connection::fd() const {
    return fd_;
}
