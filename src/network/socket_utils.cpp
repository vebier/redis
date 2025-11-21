#include "network/socket_utils.h"
#include "network/socket_error.h"
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <cstring>

namespace socket_utils {
    void set_nonblocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0) {
            throw SocketError("fcntl F_GETFL failed: " + std::string(strerror(errno)));
        }

        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
            throw SocketError("fcntl F_SETFL failed: " + std::string(strerror(errno)));
        }
    }

    int create_listening_socket(uint16_t port) {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            throw SocketError("socket() failed");
        }

        int val = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

        struct sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(fd, (const sockaddr*)&addr, sizeof(addr)) < 0) {
            close(fd);
            throw SocketError("bind() failed");
        }

        set_nonblocking(fd);

        if (listen(fd, SOMAXCONN) < 0) {
            close(fd);
            throw SocketError("listen() failed");
        }

        return fd;
    }
}
