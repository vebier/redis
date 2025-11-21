#include "server/event_loop.h"
#include "server/connection_handler.h"
#include "network/socket_error.h"
#include <poll.h>

EventLoop::EventLoop(int listen_fd) : listen_fd_(listen_fd) {}

void EventLoop::run() {
    std::vector<struct pollfd> poll_args;

    while (true) {
        poll_args.clear();
        poll_args.push_back({listen_fd_, POLLIN, 0});//进行监听事件

        for (const auto& conn : fd2conn_) {
            if (!conn) continue;

            struct pollfd pfd{conn->fd(), POLLERR, 0};
            if (conn->want_read) pfd.events |= POLLIN;
            if (conn->want_write) pfd.events |= POLLOUT;
            poll_args.push_back(pfd);
        }

        int rv = poll(poll_args.data(), poll_args.size(), -1);
        if (rv < 0 && errno == EINTR) {
            continue;
        }
        if (rv < 0) {
            throw SocketError("poll() failed");
        }

        if (poll_args[0].revents) {
            if (auto conn = ConnectionHandler::accept_connection(listen_fd_)) {
                int fd = (*conn)->fd();
                if (fd2conn_.size() <= static_cast<size_t>(fd)) {
                    fd2conn_.resize(fd + 1);
                }
                fd2conn_[fd] = std::move(*conn);
            }
        }

        for (size_t i = 1; i < poll_args.size(); ++i) {
            uint32_t ready = poll_args[i].revents;
            if (ready == 0) continue;

            Connection* conn = fd2conn_[poll_args[i].fd].get();
            if (ready & POLLIN) {
                ConnectionHandler::handle_read(*conn);
            }
            if (ready & POLLOUT) {
                ConnectionHandler::handle_write(*conn);
            }

            if ((ready & POLLERR) || conn->want_close) {
                fd2conn_[poll_args[i].fd].reset();
            }
        }
    }
}
