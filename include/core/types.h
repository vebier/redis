#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <vector>

enum class Code : uint8_t {
    TAG_NIL = 0,    // nil
    TAG_ERR = 1,    // error code + msg
    TAG_STR = 2,    // string
    TAG_INT = 3,    // int64
    TAG_DBL = 4,    // double
    TAG_ARR = 5,    // array
};

enum class Error{
    ERR_UNKNOWN = 1,    // unknown command
    ERR_TOO_BIG = 2,    // response too big
};

enum class Status : int32_t {
    OK = 0,
    ERR = 1,
    NX = 2,
};

/**
 * @brief 通过HNode成员指针反推获得Entry指针进行key值比较,首先假设一个Entry变量并且地址为0,然后获得Entry::node的偏移量,
 * 最后通过HNode指针减去偏移量获得Entry指针
 * @param ptr=结构体指针也就是HNode指针
 * @param member=HNode成员指针
 * @return T*=Entry指针
 */
template<typename M, typename T> //M*=HNode* T*=Entry*
T* container_of(M* ptr, M T::*member) {
    // 计算成员在类型中的偏移，然后回退到对象起始地址
    auto member_offset = reinterpret_cast<std::ptrdiff_t>(
            &(reinterpret_cast<T*>(0)->*member)
    );
    return reinterpret_cast<T*>(
            reinterpret_cast<char*>(ptr) - member_offset
    );
}

/**
 * @brief 计算字符串的哈希值
 * @param data=字符串指针
 * @param len=字符串长度
 * @return 哈希值
 */
static uint64_t str_hash(const uint8_t *data, size_t len) {
    uint32_t h = 0x811C9DC5;
    for (size_t i = 0; i < len; i++) {
        h = (h + data[i]) * 0x01000193;
    }
    return h;
}

#endif // TYPES_H
