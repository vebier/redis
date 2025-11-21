#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <cstdint>

namespace socket_utils {
    void set_nonblocking(int fd);
    int create_listening_socket(uint16_t port);
}

#endif // SOCKET_UTILS_H
