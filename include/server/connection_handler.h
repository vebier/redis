#ifndef CONNECTION_HANDLER_H
#define CONNECTION_HANDLER_H

#include "network/connection.h"
#include <optional>

class ConnectionHandler {
public:
    static std::optional<ConnPtr> accept_connection(int listen_fd);
    static bool try_one_request(Connection& conn);
    static void handle_read(Connection& conn);
    static void handle_write(Connection& conn);
};

#endif // CONNECTION_HANDLER_H
