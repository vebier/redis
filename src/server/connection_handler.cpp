#include "connection_handler.h"
#include "socket_utils.h"
#include "protocol_parser.h"
#include "request_handler.h"
#include "core/config.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

std::optional<ConnPtr> ConnectionHandler::accept_connection(int listen_fd) {
    struct sockaddr_in client_addr{};
    socklen_t addrlen = sizeof(client_addr);
    int connfd = accept(listen_fd, (struct sockaddr*)&client_addr, &addrlen);

    if (connfd < 0) {
        std::cerr << "accept() error: " << strerror(errno) << std::endl;
        return std::nullopt;
    }

    uint32_t ip = ntohl(client_addr.sin_addr.s_addr);
    std::cout << "New client from "
              << ((ip >> 24) & 0xFF) << "."
              << ((ip >> 16) & 0xFF) << "."
              << ((ip >> 8) & 0xFF) << "."
              << (ip & 0xFF) << ":"
              << ntohs(client_addr.sin_port) << std::endl;

    socket_utils::set_nonblocking(connfd);

    return std::make_unique<Connection>(connfd);
}

/**
 * @brief 从读取的数据中解析并处理一个请求 TVL数据包 = 总长度(4字节) + 命令行参数个数(4字节) + [参数长度(4字节) + 参数内容(可变长度)] * n
 * @param conn=客户端连接管理对象
 * @return 是否成功处理了一个请求
 * */
bool ConnectionHandler::try_one_request(Connection& conn) {
    if (conn.incoming.size() < sizeof(uint32_t)) {
        return false;
    }

    uint32_t len = 0;
    std::memcpy(&len, conn.incoming.data(), sizeof(uint32_t));

    if (len > config::MAX_MSG_SIZE) {
        std::cerr << "Message too long" << std::endl;
        conn.want_close = true;
        return false;
    }

    if (sizeof(uint32_t) + len > conn.incoming.size()) {
        return false;
    }

    const uint8_t* request = conn.incoming.data() + sizeof(uint32_t);//消息体 = 命令行参数个数(4字节) + [参数长度(4字节) + 参数内容(可变长度)] * n
    auto cmd = ProtocolParser::parse_request(request, len);

    if (!cmd) {
        std::cerr << "Bad request" << std::endl;
        conn.want_close = true;
        return false;
    }

    size_t header_pos = 0;
    conn.outgoing.response_begin(header_pos);//记录当前buffer里面有多少个字节
    RequestHandler::do_request(cmd, conn.outgoing);//进行命令行操作，然后在buffer里面插入TAG以及成功或失败的相应数据
    conn.outgoing.response_end(header_pos);//在一开始的4字节站位填充响应体总长度

    conn.incoming.consume(sizeof(uint32_t) + len);//消费已经处理完的请求数据
    return true;
}

void ConnectionHandler::handle_read(Connection& conn) {
    uint8_t buf[64 * 1024];
    ssize_t rv = read(conn.fd(), buf, sizeof(buf));

    if (rv < 0 && errno == EAGAIN) {
        return;
    }

    if (rv < 0) {
        std::cerr << "read() error: " << strerror(errno) << std::endl;
        conn.want_close = true;
        return;
    }

    if (rv == 0) {
        std::cout << (conn.incoming.empty() ? "Client closed" : "Unexpected EOF") << std::endl;
        conn.want_close = true;
        return;
    }

    conn.incoming.append(buf, static_cast<size_t>(rv));

    while (try_one_request(conn)) {}

    if (!conn.outgoing.empty()) {
        conn.want_read = false;
        conn.want_write = true;
        handle_write(conn);
    }
}

void ConnectionHandler::handle_write(Connection& conn) {
    if (conn.outgoing.empty()) {
        return;
    }

    ssize_t rv = write(conn.fd(), conn.outgoing.data(), conn.outgoing.size());

    if (rv < 0 && errno == EAGAIN) {
        return;
    }

    if (rv < 0) {
        std::cerr << "write() error: " << strerror(errno) << std::endl;
        conn.want_close = true;
        return;
    }

    conn.outgoing.consume(static_cast<size_t>(rv));

    if (conn.outgoing.empty()) {
        conn.want_read = true;
        conn.want_write = false;
    }
}
