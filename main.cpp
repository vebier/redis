#include "server/event_loop.h"
#include "network/socket_utils.h"
#include "core/config.h"
#include "storage/HMap.h"
#include "storage/AVLMap.h"
#include <iostream>
#include <string>

int main() {
    try {
        int listen_fd = socket_utils::create_listening_socket(config::SERVER_PORT);
        std::cout << "Server listening on port " << config::SERVER_PORT << std::endl;

        EventLoop loop(listen_fd);
        loop.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
