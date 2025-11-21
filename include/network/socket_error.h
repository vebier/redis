#ifndef SOCKET_ERROR_H
#define SOCKET_ERROR_H

#include <stdexcept>
#include <string>

class SocketError : public std::runtime_error {
public:
    explicit SocketError(const std::string& msg);
};

#endif // SOCKET_ERROR_H
