#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <cstdint>
#include <span>
#include <string>

class Buffer {
    std::vector<uint8_t> data_;

public:
    void append(std::span<const uint8_t> bytes);
    void append(const uint8_t* ptr, size_t len);
    void append(const uint8_t& byte);
    void consume(size_t n);

    void out_nil();
    void out_str(const std::string& s, size_t size);
    void out_int(int64_t val);
    void out_err(int32_t code, const std::string& msg);
    void out_arr(uint32_t n);

    void response_begin(size_t& header);
    size_t response_size(size_t header);
    void response_end(size_t header);

    size_t size() const;
    bool empty() const;
    const uint8_t* data() const;
    uint8_t* data();
    uint8_t operator[](size_t i) const;
    void clear();
};

#endif // BUFFER_H
