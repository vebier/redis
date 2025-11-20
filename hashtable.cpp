#include "hashtable.h"
const size_t k_max_load_factor = 8; //负载因子，每个桶的容量
const size_t k_rehashing_work = 128;//每次迁移的工作量，用于控制渐进式扩容中每次迁移的数据量

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

Htab::Htab() : tab(nullptr), mask(0), size(0) {}

Htab::~Htab() {
    if (!tab) return;
    for (size_t i = 0; i <= mask; ++i) {
        HNode* cur = tab[i];
        while (cur) {
            HNode* next = cur->next;

            auto entry = container_of(cur, &Entry<std::string, std::string>::node);
            delete entry;
            cur = next;
        }
    }
    delete[] tab;
    tab = nullptr;
}

Htab::Htab(const Htab& other): tab(nullptr), mask(0), size(0) {
    if (!other.tab) return;

    tab = new HNode*[other.mask + 1]();
    mask = other.mask;

    for (size_t i = 0; i <= other.mask; ++i) {
        HNode* cur = other.tab[i];
        HNode** dst = &tab[i];
        while (cur) {
            // 深拷贝 Entry 对象
            auto src_entry = container_of(cur, &Entry<std::string, std::string>::node);
            auto new_entry = new Entry<std::string, std::string>(*src_entry);

            *dst = &new_entry->node;
            dst = &new_entry->node.next;
            cur = cur->next;
            ++size;
        }
    }
}

Htab& Htab::operator=(const Htab& other) {
    if (this == &other) return *this;

    // 释放旧资源（使用与析构函数相同的逻辑）
    if (tab) {
        for (size_t i = 0; i <= mask; ++i) {
            HNode* cur = tab[i];
            while (cur) {
                HNode* next = cur->next;
                auto entry = container_of(cur, &Entry<std::string, std::string>::node);
                delete entry;  // 删除完整的 Entry
                cur = next;
            }
        }
        delete[] tab;
    }

    tab = nullptr;
    mask = 0;
    size = 0;

    if (!other.tab) return *this;

    tab = new HNode*[other.mask + 1]();
    mask = other.mask;

    for (size_t i = 0; i <= other.mask; ++i) {
        HNode* cur = other.tab[i];
        HNode** dst = &tab[i];
        while (cur) {
            auto src_entry = container_of(cur, &Entry<std::string, std::string>::node);
            auto new_entry = new Entry<std::string, std::string>(*src_entry);

            *dst = &new_entry->node;
            dst = &new_entry->node.next;
            cur = cur->next;
            ++size;
        }
    }

    return *this;
}

Htab::Htab(Htab&& other) noexcept
        : tab(other.tab), mask(other.mask), size(other.size) {
    other.tab = nullptr;
    other.mask = 0;
    other.size = 0;
}

Htab& Htab::operator=(Htab&& other) noexcept {
    if (this == &other) return *this;

    // 释放当前资源（与析构函数相同）
    if (tab) {
        for (size_t i = 0; i <= mask; ++i) {
            HNode* cur = tab[i];
            while (cur) {
                //  添加有效性检查
                if (!cur) break;

                HNode* next = cur->next;

                // 确保 container_of 计算正确
                auto entry = container_of(cur, &Entry<std::string, std::string>::node);

                // 检查 entry 有效性
                if (entry) {
                    delete entry;
                }

                cur = next;
            }
        }
        delete[] tab;
    }

    // 窃取资源
    tab = other.tab;
    mask = other.mask;
    size = other.size;

    other.tab = nullptr;
    other.mask = 0;
    other.size = 0;

    return *this;
}


void Htab::h_init(size_t n){
    assert(n>0&&((n-1)&n)==0);
    tab=new HNode*[n]();
    mask=n-1;
    size=0;
}

void Htab::h_insert(HNode* node){
    auto pos=node->hcode&mask;
    auto next=tab[pos];
    node->next=next;
    tab[pos]=node;
    ++size;
}

HNode** Htab::h_lookup(HNode* key,std::function<bool(HNode*,HNode*)> eq){
    if(!tab)return nullptr;
    auto pos=key->hcode&mask;
    auto from=&tab[pos];
    for(HNode* cur;(cur=*from)!= nullptr;from=&cur->next){
        if(cur->hcode==key->hcode&&eq(cur,key)){
            return from;
        }
    }
    return nullptr;
}

HNode* Htab::h_detach(HNode** from){
    if(from==nullptr)return nullptr;
    auto node=*from;
    *from=(*from)->next;
    --size;
    return node;
}

HMap::HMap():migrate_pos(0){

}

void HMap::hm_trigger_rehashing(){
    older=newer;
    newer.h_init((newer.mask+1)*2);
    migrate_pos=0;
}

void HMap::hm_help_rehashing(){
    size_t nwork=0;
    while(nwork<k_rehashing_work&&older.size>0){
        if(migrate_pos > older.mask){
            migrate_pos = 0;
        }
        auto from=&older.tab[migrate_pos];
        if(!*from){
            ++migrate_pos;
            continue;
        }
        newer.h_insert(older.h_detach(from));
        ++nwork;
    }
    if (older.size == 0 && older.tab) {
        delete[] older.tab;
        older.tab = nullptr;
        older.mask = 0;
        migrate_pos=0;
    }
}



HNode* HMap::hm_lookup(HNode* key,std::function<bool(HNode*,HNode*)> eq){
    hm_help_rehashing();
    auto from=older.h_lookup(key,eq);
    if(!from){
        from=newer.h_lookup(key,eq);
    }
    return from?*from:nullptr;
}

void HMap::hm_insert(HNode* node){
    if(!newer.tab){
        newer.h_init(4);
    }
    newer.h_insert(node);
    if (!older.tab) {
        size_t shreshold = (newer.mask + 1) * k_max_load_factor;//哈希表总节点容量
        if (newer.size >= shreshold) {
            hm_trigger_rehashing();
        }
    }
    hm_help_rehashing();
}

HNode* HMap::hm_delete(HNode* key,std::function<bool(HNode*,HNode*)> eq){
    // 先尝试查找，不触发rehashing
    if(auto from=older.h_lookup(key,eq)){
        return older.h_detach(from);
    }
    if(auto from=newer.h_lookup(key,eq)){
        return newer.h_detach(from);
    }

    // 如果没找到，再尝试触发rehashing后查找
    hm_help_rehashing();
    if(auto from=older.h_lookup(key,eq)){
        return older.h_detach(from);
    }
    if(auto from=newer.h_lookup(key,eq)){
        return newer.h_detach(from);
    }
    return nullptr;
}

void RedisDB::get(std::vector<std::string> &cmd, Response &out) {
    if (cmd.size() != 2) {
        out.status = -1;
        return;
    }
    Entry<std::string, std::string> query_entry;
    query_entry.key = cmd[1];  // 不要用 swap
    query_entry.node.hcode = str_hash(reinterpret_cast<const uint8_t*>(query_entry.key.data()),query_entry.key.size());
    auto node = db.hm_lookup(&query_entry.node, &entry_eq<Entry<std::string, std::string>>);
    if (!node) {
        out.status = 1;
        return;
    }
    auto entry = container_of(node, &Entry<std::string, std::string>::node);
    out.status = 0;
    out.data.assign(entry->val.begin(),entry->val.end());
}

void RedisDB::set(std::vector<std::string> &cmd, Response &out){
    if (cmd.size() != 3) {
        out.status = -1;
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
    out.status = 0;
}

void RedisDB::del(std::vector<std::string> &cmd, Response &out){
    if (cmd.size() != 2) {
        out.status = -1;
        return;
    }
// ✅ 创建临时 Entry 用于查询
    Entry<std::string, std::string> query_entry;
    query_entry.key = cmd[1];  // 不要用 swap
    query_entry.node.hcode = str_hash(reinterpret_cast<const uint8_t*>(query_entry.key.data()),query_entry.key.size());
    auto node = db.hm_delete(&query_entry.node, &entry_eq<Entry<std::string, std::string>>);
    if(!node){
        out.status=-1;
        return;
    }
    delete container_of(node, &Entry<std::string, std::string>::node);
    out.status = 0;
}


