#ifndef REDIS_SORTEDSET_H
#define REDIS_SORTEDSET_H

#include "HMap.h"
#include "AVLMap.h"
#include <any>
#include <string>
#include <vector>
#include <functional>

// ZSet节点结构，包含哈希表节点和AVL树节点
struct ZNode {
    HNode hnode_;    // 用于哈希表查找
    AVLNode anode_;  // 用于排序
    double score;    // 分数值
    std::string key; // 成员键

    ZNode(const std::string& k, double s) : score(s), key(k) {
        hnode_.hcode = str_hash(key); // 计算哈希值
    }
};

// ZSet条目结构，用于哈希表存储
template<typename Key, typename Value>
struct ZSetEntry : public Entry<Key, Value> {
    double score;
    ZSetEntry(const Key& key = Key{}, const Value& value = Value{}, double s = 0.0)
            : Entry<Key, Value>(key, value), score(s) {}
};

class ZSet {
public:
    ZSet();
    ~ZSet();

    // 添加或更新元素
    int add(const std::string& key, double score);

    // 删除元素
    int remove(const std::string& key);

    // 获取元素的分数
    double score(const std::string& key);

    // 获取集合大小
    size_t size() const;

    // 按分数范围获取元素
    std::vector<std::pair<std::string, double>> rangeByScore(double min, double max);

    // 按排名范围获取元素
    std::vector<std::pair<std::string, double>> rangeByRank(int start, int end);

    // 获取元素排名
    int rank(const std::string& key);

    // 增加元素分数
    double incrBy(const std::string& key, double increment);

private:
    // 比较函数，用于哈希表查找
    static bool zsetEntryEq(HNode* lhs, HNode* rhs);

    // 内部辅助函数
    ZNode* lookupZNode(const std::string& key);

    AVLMap<double, std::string> score_map_;  // 分数到键的映射，用于范围查询
    HMap key_map_;                           // 键到节点的映射，用于快速查找
    size_t size_;                            // 元素数量
};

#endif