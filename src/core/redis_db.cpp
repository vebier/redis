#include "redis_db.h"
#include "types.h"

void RedisDB::get(std::vector<std::string> &cmd, Buffer &out) {
    if (cmd.size() != 2) {
        out.out_err(1, "ERR wrong number of arguments for 'get' command");  // ✅ 返回错误
        return;
    }
    Entry<std::string, std::string> query_entry;
    query_entry.key = cmd[1];  // 不要用 swap
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
        out.out_err(1, "ERR wrong number of arguments for 'set' command");  // ✅ 返回错误
        return;
    }
    Entry<std::string, std::string> query_entry;
    query_entry.key = cmd[1];  // 不要用 swap
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
        out.out_err(1, "ERR wrong number of arguments for 'del' command");  // ✅ 返回错误
        return;
    }
// ✅ 创建临时 Entry 用于查询
    Entry<std::string, std::string> query_entry;
    query_entry.key = cmd[1];  // 不要用 swap
    query_entry.node.hcode = str_hash(reinterpret_cast<const uint8_t*>(query_entry.key.data()),query_entry.key.size());
    auto node = db.hm_delete(&query_entry.node, &entry_eq<Entry<std::string, std::string>>);
    if(node){
        delete container_of(node, &Entry<std::string, std::string>::node);;
    }

    out.out_int(node?1:0);
}

void RedisDB::keys(std::vector<std::string>& cmd, Buffer& out){
    out.out_arr(static_cast<uint32_t>(db.hm_size()));
    std::any arg = &out;  // ✅ 直接传递指针
    db.hm_foreach(&cb_keys, arg);
}

bool RedisDB::cb_keys(HNode* node, std::any& arg){
    Buffer* out = std::any_cast<Buffer*>(arg);  // ✅ 提取指针
    const std::string &key = container_of(node,&Entry<std::string, std::string>::node)->key;
    out->out_str(key, key.size());
    return true;
}