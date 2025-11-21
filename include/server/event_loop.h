#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include "network/connection.h"
#include <vector>

class EventLoop {
    int listen_fd_;
    std::vector<ConnPtr> fd2conn_;

public:
    explicit EventLoop(int listen_fd);
    void run();
};

#endif // EVENT_LOOP_H
