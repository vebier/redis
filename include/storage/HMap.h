#ifndef REDIS_HMAP_H
#define REDIS_HMAP_H
#include "buffer.h"
#include <cassert>
#include <functional>
#include <string>
#include <any>



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

/**
 * @brief 比较两个HNode指针指向的Entry结构体的key值是否相等
 * @param lhs=左操作数HNode指针
 * @param rhs=右操作数HNode指针
 * @return key值是否相等
 */
template<typename EntryType>
static bool entry_eq(HNode *lhs, HNode *rhs) {
    EntryType *le = container_of(lhs, &EntryType::node);
    EntryType *re = container_of(rhs, &EntryType::node);
    return le->key == re->key;
}

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

    /**
     * @brief 遍历哈希表所有节点，调用回调函数
     * @param func 回调函数，返回false则停止遍历
     */
    bool h_foreach(const std::function<bool(HNode*, Buffer&)>& func, Buffer& arg);

    void h_clear();
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

    /**
     * @brief 清空哈希表
     */
    void hm_clear();

    /**
     * @brief 获取哈希表节点数量
     * @return 哈希表节点数量
     */
    size_t hm_size();

    /**
     * @brief 遍历哈希表所有节点，调用回调函数
     * @param func 回调函数，返回false则停止遍历
     */
    void hm_foreach(const std::function<bool(HNode*, Buffer&)>& func, Buffer& arg);


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





#endif //REDIS_HMAP_H
