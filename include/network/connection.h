#ifndef CONNECTION_H
#define CONNECTION_H

#include "buffer.h"
#include <memory>

class Connection {
    int fd_;

public:
    bool want_read = false;
    bool want_write = false;
    bool want_close = false;
    Buffer incoming;
    Buffer outgoing;

    explicit Connection(int fd);
    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(Connection&& other) noexcept;
    Connection& operator=(Connection&& other) noexcept = default;

    int fd() const;
};

using ConnPtr = std::unique_ptr<Connection>;

#endif // CONNECTION_H
