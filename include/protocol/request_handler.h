#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "types.h"
#include "buffer.h"
#include <string>
#include <vector>
#include <optional>

class RequestHandler {
public:
    static void do_request(std::optional<std::vector<std::string>> cmd, Buffer& out);
};

#endif // REQUEST_HANDLER_H
