#ifndef PROTOCOL_PARSER_H
#define PROTOCOL_PARSER_H

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

class ProtocolParser {
public:
    static bool read_u32(const uint8_t*& cur, const uint8_t* end, uint32_t& out);
    static bool read_str(const uint8_t*& cur, const uint8_t* end, size_t n, std::string& out);
    static std::optional<std::vector<std::string>> parse_request(const uint8_t* data, size_t size);
};

#endif // PROTOCOL_PARSER_H
