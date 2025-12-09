#ifndef REDIS_REDIS_DB_H
#define REDIS_REDIS_DB_H

#include "HMap.h"
#include "buffer.h"
#include "singleton.h"
#include "storage/SortedSet.h"
#include <any>

class RedisDB:public Singleton<RedisDB> {
    friend class Singleton<RedisDB>;
public:
    ~RedisDB() = default;
    RedisDB(const RedisDB&)=delete;
    RedisDB& operator=(const RedisDB&)=delete;
    RedisDB(RedisDB&&)=delete;
    RedisDB& operator=(RedisDB&&)=delete;

    // 获取键值
    void get(std::vector<std::string> &cmd, Buffer &out);
    // 设置键值
    void set(std::vector<std::string> &cmd, Buffer &out);
    // 删除键
    void del(std::vector<std::string> &cmd, Buffer &out);

    // ZSet相关命令
    void zadd(std::vector<std::string> &cmd, Buffer &out);
    void zrem(std::vector<std::string> &cmd, Buffer &out);
    void zscore(std::vector<std::string> &cmd, Buffer &out);
    void zcard(std::vector<std::string> &cmd, Buffer &out);
    void zrange(std::vector<std::string> &cmd, Buffer &out);

    void keys(std::vector<std::string>& cmd, Buffer& out);

private:
    RedisDB() = default;
    // 提取节点key值写道输出缓冲区
    static bool cb_keys(HNode* node, Buffer& out);

    // ZSet相关辅助函数
    static bool cb_zset_keys(HNode* node, Buffer& out);

private:
    HMap db;
};

#endif //REDIS_REDIS_DB_H