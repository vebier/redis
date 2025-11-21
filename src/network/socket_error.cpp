#include "network/socket_error.h"

SocketError::SocketError(const std::string& msg)
        : std::runtime_error(msg) {}
