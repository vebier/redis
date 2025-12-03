#ifndef REDIS_REDIS_DB_H
#define REDIS_REDIS_DB_H

#include "HMap.h"
#include "buffer.h"
#include "singleton.h"
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

    void keys(std::vector<std::string>& cmd, Buffer& out);

private:
    RedisDB() = default;
    // 提取节点key值写道输出缓冲区
    static bool cb_keys(HNode* node, Buffer& out);
private:
    HMap db;
};
#endif //REDIS_REDIS_DB_H
