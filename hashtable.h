#ifndef REDIS_HASHTABLE_H
#define REDIS_HASHTABLE_H
#include <stdint.h>
#include <cassert>
#include <functional>
#include <string>

// 哈希表节点，应该被嵌入到数据负载中
struct HNode {
    HNode*      next;
    uint64_t    hcode;
    HNode():next(nullptr),hcode(0){}
    ~HNode() = default;
    // 拷贝构造函数
    HNode(const HNode& other) : next(nullptr), hcode(other.hcode) {
        // 注意：next指针不复制，因为新节点将插入到新的链表中
    }

    // 拷贝赋值运算符
    HNode& operator=(const HNode& other) {
        if (this != &other) {
            hcode = other.hcode;
            // next指针不复制，保持原样
        }
        return *this;
    }

    // 移动构造函数
    HNode(HNode&& other) noexcept : next(other.next), hcode(other.hcode) {
        other.next = nullptr;
        other.hcode = 0;
    }

    // 移动赋值运算符
    HNode& operator=(HNode&& other) noexcept {
        if (this != &other) {
            next = other.next;
            hcode = other.hcode;

            other.next = nullptr;
            other.hcode = 0;
        }
        return *this;
    }

};

struct Htab{
    HNode** tab;
    size_t  mask;//哈希表槽位掩码，+1就是容量
    size_t  size;//当前哈希表节点数量
    Htab();
    ~Htab();
    Htab(const Htab& other);
    Htab& operator=(const Htab& other);
    Htab(Htab&& other) noexcept;
    Htab& operator=(Htab&& other) noexcept;
    /**
    * @brief 哈希表槽位初始化
    * @param n 槽位个数
    */
    void    h_init(size_t n);
    /**
     * @brief 节点前端插入
     * @param node 要插入的节点指针
     */
    void    h_insert(HNode* node);
    /**
     * @brief 查找KEY节点指针的指针
     * @param key 查找节点
     * @param eq 相性比较回调函数
     * @return 返回父指针的地址，无需单独的查找和删除函数
     */
    HNode** h_lookup(HNode* key,std::function<bool(HNode*,HNode*)> eq);
    /**
     * @brief 删除节点
     * @param from 删除节点
     * @return 返回已经从哈希表中删除的节点指针
     */
    HNode*  h_detach(HNode** from);
};

class HMap{
public:
    HMap();
    /**
     * @brief 查找KEY节点
     * @param key 查找节点
     * @param eq 相性比较回调函数
     * @return 返回HNode指针，无需单独的查找和删除函数
     */
    HNode*  hm_lookup(HNode* key,std::function<bool(HNode*,HNode*)> eq);
    /**
     * @brief 节点前端插入
     * @param node 要插入的节点指针
     */
    void    hm_insert(HNode* node);
    /**
     * @brief 删除节点
     * @param key 删除节点
     * @param eq 相性比较回调函数
     * @return 返回已经从哈希表中删除的节点指针
     */
    HNode*  hm_delete(HNode* key,std::function<bool(HNode*,HNode*)> eq);
private:
    /**
     * @brief 触发哈希表渐进式扩容
     */
    void hm_trigger_rehashing();
    /**
     * @brief 数据迁移，迁移older哈希表中的节点到newer哈希表，迁移最大128个节点
     */
    void hm_help_rehashing();
private:
    Htab    newer;
    Htab    older;
    size_t  migrate_pos;
};

template<typename Key,typename Value>
struct Entry {
    Key key;
    Value val;
    HNode node;
};
class Response;

class RedisDB {
public:
    RedisDB() = default;
    ~RedisDB() = default;

    // 获取键值
    void get(std::vector<std::string> &cmd, Response &out);
    // 设置键值
    void set(std::vector<std::string> &cmd, Response &out);
    // 删除键
    void del(std::vector<std::string> &cmd, Response &out);

private:
    HMap db;  // 使用哈希表存储数据
};

struct Response {
    int32_t status = 0;
    std::vector<uint8_t> data;
};

#endif //REDIS_HASHTABLE_H
