#include <cmath>
#include "redis_db.h"
#include "types.h"
#include "storage/SortedSet.h"

void RedisDB::get(std::vector<std::string> &cmd, Buffer &out) {
    if (cmd.size() != 2) {
        out.out_err(1, "ERR wrong number of arguments for 'get' command");
        return;
    }
    Entry<std::string, std::string> query_entry;
    query_entry.key = cmd[1];
    query_entry.node.hcode = str_hash(reinterpret_cast<const uint8_t*>(query_entry.key.data()),query_entry.key.size());
    auto node = db.hm_lookup(&query_entry.node, &entry_eq<Entry<std::string, std::string>>);
    if (!node) {
        out.out_nil();
        return;
    }
    auto entry = container_of(node, &Entry<std::string, std::string>::node);
    out.out_str(entry->val,entry->val.size());
}

void RedisDB::set(std::vector<std::string> &cmd, Buffer &out){
    if (cmd.size() != 3) {
        out.out_err(1, "ERR wrong number of arguments for 'set' command");
        return;
    }
    Entry<std::string, std::string> query_entry;
    query_entry.key = cmd[1];
    query_entry.node.hcode = str_hash(reinterpret_cast<const uint8_t*>(query_entry.key.data()),query_entry.key.size());
    auto node = db.hm_lookup(&query_entry.node, &entry_eq<Entry<std::string, std::string>>);
    if(node){
        container_of(node, &Entry<std::string, std::string>::node)->val.swap(cmd[2]);
    }else{
        auto ent=new Entry<std::string,std::string>();
        ent->node.hcode=query_entry.node.hcode;
        ent->key=query_entry.key;
        ent->val.swap(cmd[2]);
        db.hm_insert(&ent->node);
    }
    out.out_str("OK", 2);
}

void RedisDB::del(std::vector<std::string> &cmd, Buffer &out){
    if (cmd.size() != 2) {
        out.out_err(1, "ERR wrong number of arguments for 'del' command");
        return;
    }
    Entry<std::string, std::string> query_entry;
    query_entry.key = cmd[1];
    query_entry.node.hcode = str_hash(reinterpret_cast<const uint8_t*>(query_entry.key.data()),query_entry.key.size());
    auto node = db.hm_delete(&query_entry.node, &entry_eq<Entry<std::string, std::string>>);
    if(node){
        delete container_of(node, &Entry<std::string, std::string>::node);;
    }

    out.out_int(node?1:0);
}

// ZADD key score member [score member ...]
void RedisDB::zadd(std::vector<std::string> &cmd, Buffer &out) {
    if (cmd.size() < 4 || cmd.size() % 2 != 0) {
        out.out_err(1, "ERR wrong number of arguments for 'zadd' command");
        return;
    }

    std::string key = cmd[1];

    // 查找是否已存在ZSet
    Entry<std::string, std::any> query_entry;
    query_entry.key = key;
    query_entry.node.hcode = str_hash(reinterpret_cast<const uint8_t*>(key.data()), key.size());

    auto node = db.hm_lookup(&query_entry.node, &entry_eq<Entry<std::string, std::any>>);
    ZSet* zset = nullptr;

    if (node) {
        // 检查现有条目是否为ZSet类型
        auto entry = container_of(node, &Entry<std::string, std::any>::node);
        try {
            zset = std::any_cast<ZSet*>(entry->val);
        } catch (const std::bad_any_cast& e) {
            out.out_err(1, "ERR Existing key has wrong type");
            return;
        }
    } else {
        // 创建新的ZSet
        zset = new ZSet();
        auto ent = new Entry<std::string, std::any>();
        ent->node.hcode = query_entry.node.hcode;
        ent->key = key;
        ent->val = zset;
        db.hm_insert(&ent->node);
    }

    // 添加元素
    int added = 0;
    for (size_t i = 2; i < cmd.size(); i += 2) {
        try {
            double score = std::stod(cmd[i]);
            std::string member = cmd[i + 1];
            added += zset->add(member, score);
        } catch (const std::exception& e) {
            out.out_err(1, "ERR value is not a valid float");
            return;
        }
    }

    out.out_int(added);
}

// ZREM key member [member ...]
void RedisDB::zrem(std::vector<std::string> &cmd, Buffer &out) {
    if (cmd.size() < 3) {
        out.out_err(1, "ERR wrong number of arguments for 'zrem' command");
        return;
    }

    std::string key = cmd[1];

    // 查找ZSet
    Entry<std::string, std::any> query_entry;
    query_entry.key = key;
    query_entry.node.hcode = str_hash(reinterpret_cast<const uint8_t*>(key.data()), key.size());

    auto node = db.hm_lookup(&query_entry.node, &entry_eq<Entry<std::string, std::any>>);
    if (!node) {
        out.out_int(0);
        return;
    }

    ZSet* zset = nullptr;
    try {
        auto entry = container_of(node, &Entry<std::string, std::any>::node);
        zset = std::any_cast<ZSet*>(entry->val);
    } catch (const std::bad_any_cast& e) {
        out.out_err(1, "ERR key has wrong type");
        return;
    }

    // 删除元素
    int removed = 0;
    for (size_t i = 2; i < cmd.size(); i++) {
        removed += zset->remove(cmd[i]);
    }

    out.out_int(removed);
}

// ZSCORE key member
void RedisDB::zscore(std::vector<std::string> &cmd, Buffer &out) {
    if (cmd.size() != 3) {
        out.out_err(1, "ERR wrong number of arguments for 'zscore' command");
        return;
    }

    std::string key = cmd[1];
    std::string member = cmd[2];

    // 查找ZSet
    Entry<std::string, std::any> query_entry;
    query_entry.key = key;
    query_entry.node.hcode = str_hash(reinterpret_cast<const uint8_t*>(key.data()), key.size());

    auto node = db.hm_lookup(&query_entry.node, &entry_eq<Entry<std::string, std::any>>);
    if (!node) {
        out.out_nil();
        return;
    }

    ZSet* zset = nullptr;
    try {
        auto entry = container_of(node, &Entry<std::string, std::any>::node);
        zset = std::any_cast<ZSet*>(entry->val);
    } catch (const std::bad_any_cast& e) {
        out.out_nil();
        return;
    }

    double score = zset->score(member);
    if (std::isnan(score)) {
        out.out_nil();
    } else {
        out.out_str(std::to_string(score), std::to_string(score).size());
    }
}

// ZCARD key
void RedisDB::zcard(std::vector<std::string> &cmd, Buffer &out) {
    if (cmd.size() != 2) {
        out.out_err(1, "ERR wrong number of arguments for 'zcard' command");
        return;
    }

    std::string key = cmd[1];

    // 查找ZSet
    Entry<std::string, std::any> query_entry;
    query_entry.key = key;
    query_entry.node.hcode = str_hash(reinterpret_cast<const uint8_t*>(key.data()), key.size());

    auto node = db.hm_lookup(&query_entry.node, &entry_eq<Entry<std::string, std::any>>);
    if (!node) {
        out.out_int(0);
        return;
    }

    ZSet* zset = nullptr;
    try {
        auto entry = container_of(node, &Entry<std::string, std::any>::node);
        zset = std::any_cast<ZSet*>(entry->val);
    } catch (const std::bad_any_cast& e) {
        out.out_int(0);
        return;
    }

    out.out_int(zset->size());
}

// ZRANGE key start stop [WITHSCORES]
void RedisDB::zrange(std::vector<std::string> &cmd, Buffer &out) {
    if (cmd.size() < 4 || cmd.size() > 5) {
        out.out_err(1, "ERR wrong number of arguments for 'zrange' command");
        return;
    }

    std::string key = cmd[1];
    bool with_scores = (cmd.size() == 5 && (cmd[4] == "WITHSCORES" || cmd[4] == "withscores"));

    int start, stop;
    try {
        start = std::stoi(cmd[2]);
        stop = std::stoi(cmd[3]);
    } catch (const std::exception& e) {
        out.out_err(1, "ERR value is not an integer or out of range");
        return;
    }

    // 查找ZSet
    Entry<std::string, std::any> query_entry;
    query_entry.key = key;
    query_entry.node.hcode = str_hash(reinterpret_cast<const uint8_t*>(key.data()), key.size());

    auto node = db.hm_lookup(&query_entry.node, &entry_eq<Entry<std::string, std::any>>);
    if (!node) {
        out.out_arr(0);
        return;
    }

    ZSet* zset = nullptr;
    try {
        auto entry = container_of(node, &Entry<std::string, std::any>::node);
        zset = std::any_cast<ZSet*>(entry->val);
    } catch (const std::bad_any_cast& e) {
        out.out_arr(0);
        return;
    }

    auto result = zset->rangeByRank(start, stop);
    size_t count = with_scores ? result.size() * 2 : result.size();
    out.out_arr(static_cast<uint32_t>(count));

    for (const auto& item : result) {
        out.out_str(item.first, item.first.size());
        if (with_scores) {
            std::string score_str = std::to_string(item.second);
            out.out_str(score_str, score_str.size());
        }
    }
}

void RedisDB::keys(std::vector<std::string>& cmd, Buffer& out){
    out.out_arr(static_cast<uint32_t>(db.hm_size()));// ✅ 直接传递指针
    db.hm_foreach(&cb_keys, out);
}

bool RedisDB::cb_keys(HNode* node, Buffer& out){
    const std::string &key = container_of(node,&Entry<std::string, std::any>::node)->key;
    out.out_str(key, key.size());
    return true;
}

bool RedisDB::cb_zset_keys(HNode* node, Buffer& out){
    const std::string &key = container_of(node,&Entry<std::string, std::any>::node)->key;
    out.out_str(key, key.size());
    return true;
}