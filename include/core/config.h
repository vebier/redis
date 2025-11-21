#ifndef CONFIG_H
#define CONFIG_H

#include <cstddef>
#include <cstdint>

namespace config {
    constexpr size_t MAX_MSG_SIZE = 32 << 20;
    constexpr size_t MAX_ARGS = 200'000;
    constexpr uint16_t SERVER_PORT = 1234;
    constexpr const size_t k_max_load_factor = 8; //负载因子，每个桶的容量
    constexpr const size_t k_rehashing_work = 128;//每次迁移的工作量，用于控制渐进式扩容中每次迁移的数据量
}

#endif // CONFIG_H
