#include "buffer.h"
#include "types.h"
#include "config.h"
#include <stdexcept>

void Buffer::append(std::span<const uint8_t> bytes) {
    data_.insert(data_.end(), bytes.begin(), bytes.end());
}

void Buffer::append(const uint8_t* ptr, size_t len) {
    data_.insert(data_.end(), ptr, ptr + len);
}

void Buffer::append(const uint8_t& byte){
    data_.emplace_back(byte);
}

void Buffer::consume(size_t n) {
    if (n > data_.size()) {
        throw std::out_of_range("Buffer::consume: n > size");
    }
    data_.erase(data_.begin(), data_.begin() + n);
}

void Buffer::out_nil() {
    uint8_t code = static_cast<uint8_t>(Code::TAG_NIL);
    append(code);
}

void Buffer::out_str(const std::string& s, size_t size) {
    uint8_t code = static_cast<uint8_t>(Code::TAG_STR);
    append(code);

    uint32_t len = static_cast<uint32_t>(size);
    append(reinterpret_cast<const uint8_t*>(&len), sizeof(len));
    append(reinterpret_cast<const uint8_t*>(s.c_str()), size);
}

void Buffer::out_int(int64_t val) {
    uint8_t code = static_cast<uint8_t>(Code::TAG_INT);
    append(code);
    append(reinterpret_cast<const uint8_t*>(&val), sizeof(val));
}

void Buffer::out_err(int32_t code, const std::string& msg) {
    uint8_t type = static_cast<uint8_t>(Code::TAG_ERR);
    append(type);
    append(reinterpret_cast<const uint8_t*>(&code), sizeof(code));

    uint32_t len = static_cast<uint32_t>(msg.size());
    append(reinterpret_cast<const uint8_t*>(&len), sizeof(len));
    append(reinterpret_cast<const uint8_t*>(msg.data()), msg.size());
}

void Buffer::out_arr(uint32_t n) {
    uint8_t code = static_cast<uint8_t>(Code::TAG_ARR);
    append(code);
    append(reinterpret_cast<const uint8_t*>(&n), sizeof(n));
}

void Buffer::response_begin(size_t& header){
    header = data_.size();
    uint32_t placeholder = 0;
    append(reinterpret_cast<const uint8_t*>(&placeholder), 4);  // ✅ 占位 4 字节
}

size_t Buffer::response_size(size_t header){
    return data_.size()-header-4; //当前数组大小减去占位的4字节和插入数据之前的大小得到当前操作插入数据的大小
}

void Buffer::response_end(size_t header){
    size_t msg_size=response_size(header);
    if (msg_size > config::MAX_MSG_SIZE) {
        data_.resize(header + 4);
        out_err(static_cast<int32_t>(Error::ERR_TOO_BIG), "response is too big.");
        msg_size = response_size(header);
    }
    uint32_t len = (uint32_t)msg_size;
    const auto* bytes = reinterpret_cast<const uint8_t*>(&len);
    data_[header + 0] = bytes[0];
    data_[header + 1] = bytes[1];
    data_[header + 2] = bytes[2];
    data_[header + 3] = bytes[3];

}

size_t Buffer::size() const {
    return data_.size();
}

bool Buffer::empty() const {
    return data_.empty();
}

const uint8_t* Buffer::data() const {
    return data_.data();
}

uint8_t* Buffer::data() {
    return data_.data();
}

uint8_t Buffer::operator[](size_t i) const {
    return data_[i];
}

void Buffer::clear() {
    data_.clear();
}
