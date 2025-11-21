#include "request_handler.h"
#include "redis_db.h"
#include "types.h"

void RequestHandler::do_request(std::optional<std::vector<std::string>> cmd, Buffer& out) {
    if (cmd->size() == 2 && (*cmd)[0] == "get") {
        RedisDB::instance()->get(*cmd, out);
    } else if (cmd->size() == 3 && (*cmd)[0] == "set") {
        RedisDB::instance()->set(*cmd, out);
    } else if (cmd->size() == 2 && (*cmd)[0] == "del") {
        RedisDB::instance()->del(*cmd, out);
    } else {
        out.out_err(static_cast<int32_t>(Error::ERR_UNKNOWN), "unknown command.");
    }
}


