#include "SortedSet.h"
#include <algorithm>
#include <limits>

ZSet::ZSet() : size_(0) {}

ZSet::~ZSet() {
    // 清理资源
    Buffer dummy;
    key_map_.hm_foreach([](HNode* node, Buffer& buf) -> bool {
        ZNode* znode = container_of(node, &ZNode::hnode_);
        delete znode;
        return true;
    }, dummy);
}

bool ZSet::zsetEntryEq(HNode* lhs, HNode* rhs) {
    ZNode* l = container_of(lhs, &ZNode::hnode_);
    ZNode* r = container_of(rhs, &ZNode::hnode_);
    return l->key == r->key;
}

ZNode* ZSet::lookupZNode(const std::string& key) {
    // 创建临时节点用于查找
    ZNode tmp(key, 0.0);
    HNode* found = key_map_.hm_lookup(&tmp.hnode_, zsetEntryEq);
    return found ? container_of(found, &ZNode::hnode_) : nullptr;
}

int ZSet::add(const std::string& key, double score) {
    ZNode* znode = lookupZNode(key);

    if (znode) {
        // 如果节点已存在，更新分数
        if (znode->score == score) {
            return 0; // 分数未改变
        }

        // 从AVL树中删除旧节点
        score_map_.remove(znode->score);

        // 更新分数
        znode->score = score;

        // 插入新节点到AVL树
        score_map_.put(score, key);

        return 0; // 更新成功
    } else {
        // 创建新节点
        znode = new ZNode(key, score);

        // 插入到哈希表
        key_map_.hm_insert(&znode->hnode_);

        // 插入到AVL树
        score_map_.put(score, key);

        size_++;
        return 1; // 新增成功
    }
}

int ZSet::remove(const std::string& key) {
    ZNode* znode = lookupZNode(key);
    if (!znode) {
        return 0; // 元素不存在
    }

    // 从哈希表中删除
    key_map_.hm_delete(&znode->hnode_, zsetEntryEq);

    // 从AVL树中删除
    score_map_.remove(znode->score);

    // 释放内存
    delete znode;

    size_--;
    return 1; // 删除成功
}

double ZSet::score(const std::string& key) {
    ZNode* znode = lookupZNode(key);
    return znode ? znode->score : std::numeric_limits<double>::quiet_NaN();
}

size_t ZSet::size() const {
    return size_;
}

std::vector<std::pair<std::string, double>> ZSet::rangeByScore(double min, double max) {
    std::vector<std::pair<std::string, double>> result;

    // 获取所有元素并过滤范围内的元素
    auto all_elements = score_map_.inorderTraversal();

    for (const auto& element : all_elements) {
        double score = element.first;
        const std::string& key = element.second;

        if (score >= min && score <= max) {
            result.emplace_back(key, score);
        }
    }

    return result;
}

std::vector<std::pair<std::string, double>> ZSet::rangeByRank(int start, int end) {
    std::vector<std::pair<std::string, double>> result;

    // 获取所有元素
    auto all_elements = score_map_.inorderTraversal();

    // 处理负索引
    if (start < 0) start = std::max(0, static_cast<int>(all_elements.size()) + start);
    if (end < 0) end = std::max(0, static_cast<int>(all_elements.size()) + end);

    // 确保索引有效
    start = std::max(0, start);
    end = std::min(static_cast<int>(all_elements.size()) - 1, end);

    if (start <= end && start < static_cast<int>(all_elements.size())) {
        for (int i = start; i <= end && i < static_cast<int>(all_elements.size()); ++i) {
            const auto& element = all_elements[i];
            result.emplace_back(element.second, element.first);
        }
    }

    return result;
}

int ZSet::rank(const std::string& key) {
    ZNode* znode = lookupZNode(key);
    if (!znode) {
        return -1; // 元素不存在
    }

    // 获取所有元素并查找排名
    auto all_elements = score_map_.inorderTraversal();

    for (size_t i = 0; i < all_elements.size(); ++i) {
        if (all_elements[i].second == key) {
            return static_cast<int>(i);
        }
    }

    return -1; // 未找到
}

double ZSet::incrBy(const std::string& key, double increment) {
    ZNode* znode = lookupZNode(key);
    if (!znode) {
        // 如果元素不存在，则添加新元素，分数为increment
        add(key, increment);
        return increment;
    }

    // 更新分数
    double new_score = znode->score + increment;
    add(key, new_score); // 使用add方法更新分数

    return new_score;
}