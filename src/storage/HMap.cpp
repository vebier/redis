#include "types.h"
#include "config.h"
#include "HMap.h"

Htab::Htab() : tab(nullptr), mask(0), size(0) {}

Htab::~Htab() {
    h_clear();
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
    h_clear();

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
    h_clear();

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

bool Htab::h_foreach(const std::function<bool(HNode*, std::any&)>& func,std::any& arg){
    for(size_t i=0; mask!=0 && i<=mask; ++i){
        for(auto node=tab[i]; node!=nullptr; node=node->next){
            if(!func(node, arg)){
                return false;
            }
        }
    }
    return true;
}

void Htab::h_clear(){
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
    mask=0;
    size=0;
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
    while(nwork<config::k_rehashing_work&&older.size>0){
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
        size_t shreshold = (newer.mask + 1) * config::k_max_load_factor;//哈希表总节点容量
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

void HMap::hm_clear(){
    newer.h_clear();  // 使用默认构造重置
    older.h_clear();
    migrate_pos = 0;
}

size_t HMap::hm_size(){
    return older.size + newer.size;
}

void HMap::hm_foreach(const std::function<bool(HNode*, std::any&)>& func, std::any& arg){
    this->older.h_foreach(func, arg) && this->newer.h_foreach(func, arg);
}

