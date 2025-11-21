#include "protocol/protocol_parser.h"
#include "core/config.h"
#include <cstring>

bool ProtocolParser::read_u32(const uint8_t*& cur, const uint8_t* end, uint32_t& out) {
    if (cur + sizeof(uint32_t) > end) {
        return false;
    }
    std::memcpy(&out, cur, sizeof(uint32_t));
    cur += sizeof(uint32_t);
    return true;
}

bool ProtocolParser::read_str(const uint8_t*& cur, const uint8_t* end, size_t n, std::string& out) {
    if (cur + n > end) {
        return false;
    }
    out.assign(cur, cur + n);
    cur += n;
    return true;
}

std::optional<std::vector<std::string>> ProtocolParser::parse_request(const uint8_t* data, size_t size) {
    const uint8_t* end = data + size;
    uint32_t nstr = 0;//从命令行 string个数 比如 get key  nstr=2

    if (!read_u32(data, end, nstr) || nstr > config::MAX_ARGS) {
        return std::nullopt;
    }

    std::vector<std::string> result;
    result.reserve(nstr);

    while (result.size() < nstr) {
        uint32_t len = 0;
        if (!read_u32(data, end, len)) {
            return std::nullopt;
        }

        result.emplace_back();
        if (!read_str(data, end, len, result.back())) {
            return std::nullopt;
        }
    }

    if (data != end) {
        return std::nullopt;
    }

    return result;
}
