#include "request_handler.h"
#include "redis_db.h"
#include "types.h"

void RequestHandler::do_request(std::optional<std::vector<std::string>> cmd, Buffer& out) {
    if (!cmd.has_value()) {
        out.out_err(static_cast<int32_t>(Error::ERR_UNKNOWN), "empty command.");
        return;
    }

    if (cmd->empty()) {
        out.out_err(static_cast<int32_t>(Error::ERR_UNKNOWN), "empty command.");
        return;
    }

    const std::string& command = (*cmd)[0];

    // 字符串命令
    if (cmd->size() == 2 && command == "get") {
        RedisDB::instance()->get(*cmd, out);
    } else if (cmd->size() == 3 && command == "set") {
        RedisDB::instance()->set(*cmd, out);
    } else if (cmd->size() == 2 && command == "del") {
        RedisDB::instance()->del(*cmd, out);
    }
        // ZSet命令
    else if (command == "zadd" && cmd->size() >= 4) {
        RedisDB::instance()->zadd(*cmd, out);
    } else if (command == "zrem" && cmd->size() >= 3) {
        RedisDB::instance()->zrem(*cmd, out);
    } else if (command == "zscore" && cmd->size() == 3) {
        RedisDB::instance()->zscore(*cmd, out);
    } else if (command == "zcard" && cmd->size() == 2) {
        RedisDB::instance()->zcard(*cmd, out);
    } else if ((command == "zrange" && (cmd->size() == 4 || cmd->size() == 5))) {
        RedisDB::instance()->zrange(*cmd, out);
    } else {
        out.out_err(static_cast<int32_t>(Error::ERR_UNKNOWN), "unknown command.");
    }
}


