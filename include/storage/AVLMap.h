// c++
#ifndef REDIS_AVLMAP_H
#define REDIS_AVLMAP_H
#include "types.h"

#include <cstdint>
#include <algorithm>
#include <vector>
#include <functional>
#include <iostream>

struct AVLNode{
    AVLNode* parent_;
    AVLNode* right_;
    AVLNode* left_;
    uint32_t height_;
    AVLNode();
};

template<typename Key,typename Value>
struct Data{
    Key key_;
    Value value_;
    AVLNode node_;
    Data(const Key& key=Key{}, const Value& value=Value{}):key_(key),value_(value){}
};

static inline int get_height(AVLNode* node){
    return node?node->height_:0;
}

static inline int getBalance(AVLNode* node){
    if(!node) return 0;
    return get_height(node->left_)-get_height(node->right_);
}

static inline AVLNode* rot_right(AVLNode* node){
    auto parent=node->parent_;
    auto new_node=node->left_;
    auto inner=new_node->right_;
    if(inner){
        inner->parent_=node;
        node->left_=inner;
    }else{
        node->left_=nullptr;
    }
    new_node->parent_=parent;
    new_node->right_=node;
    node->parent_=new_node;
    node->height_=std::max(get_height(node->left_), get_height(node->right_))+1;
    new_node->height_=std::max(get_height(new_node->left_), get_height(new_node->right_))+1;
    return new_node;
}

static inline AVLNode* rot_left(AVLNode* node){
    auto parent=node->parent_;
    auto new_node=node->right_;
    auto inner=new_node->left_;
    if(inner){
        inner->parent_=node;
        node->right_=inner;
    }else{
        node->right_=nullptr;
    }
    new_node->parent_=parent;
    new_node->left_=node;
    node->parent_=new_node;
    node->height_=std::max(get_height(node->left_), get_height(node->right_))+1;
    new_node->height_=std::max(get_height(new_node->left_), get_height(new_node->right_))+1;
    return new_node;
}

template<typename Key,typename Value>
static AVLNode* insert(AVLNode* node, const Key& key, const Value& value, AVLNode* parent = nullptr) {
    if (node == nullptr) {
        auto data = new Data<Key, Value>(key, value);
        return &(data->node_);
    }

    auto data = container_of(node, &Data<Key, Value>::node_);
//    std::cout << "Current node key: " << data->key_ << ", Inserting key: " << key << std::endl;
//    if (node->left_) {
//        auto left_data = container_of(node->left_, &Data<Key,Value>::node_);
//        std::cout << "Left child key: " << left_data->key_ << std::endl;
//    }
//    if (node->right_) {
//        auto right_data = container_of(node->right_, &Data<Key,Value>::node_);
//        std::cout << "Right child key: " << right_data->key_ << std::endl;
//    }

    if (key < data->key_) {
        node->left_ = insert(node->left_, key, value, node);
    } else if (key > data->key_) {
        node->right_ = insert(node->right_, key, value, node);
    } else {
        data->value_ = value;
        return node;
    }

    node->height_ = 1 + std::max(get_height(node->left_), get_height(node->right_));
    int balance = getBalance(node);

    // LL
    if (balance > 1) {
        auto left_data = node->left_ ? container_of(node->left_, &Data<Key, Value>::node_) : nullptr;
        if (left_data && key < left_data->key_) {
            return rot_right(node);
        }
        // LR
        if (left_data && key > left_data->key_) {
            node->left_ = rot_left(node->left_);
            return rot_right(node);
        }
    }

    // RR
    if (balance < -1) {
        auto right_data = node->right_ ? container_of(node->right_, &Data<Key, Value>::node_) : nullptr;
        if (right_data && key > right_data->key_) {
            return rot_left(node);
        }
        // RL
        if (right_data && key < right_data->key_) {
            node->right_ = rot_right(node->right_);
            return rot_left(node);
        }
    }

    return node;
}

template<typename Key,typename Value>
static Value* search(AVLNode* node,const Key& key){
    if(!node) return nullptr;
    auto data=container_of(node,&Data<Key,Value>::node_);
    if(key<data->key_) return search<Key,Value>(node->left_,key);
    if(key>data->key_) return search<Key,Value>(node->right_,key);
    return &data->value_;
}

template<typename Key,typename Value>
static AVLNode* get_mini_node(AVLNode* node){
    auto current=node;
    while(current && current->left_) current=current->left_;
    return current;
}

template<typename Key,typename Value>
static AVLNode* deleteNode(AVLNode* root,const Key& key){
    if(root==nullptr) return root;
    auto data=container_of(root,&Data<Key,Value>::node_);
    if(key<data->key_){
        root->left_ = deleteNode<Key,Value>(root->left_,key);
    }else if(key>data->key_){
        root->right_ = deleteNode<Key,Value>(root->right_,key);
    }else{
        if(!root->left_ || !root->right_){
            AVLNode* child = root->left_ ? root->left_ : root->right_;
            if(!child){
                delete data;
                return nullptr;
            }else{
                auto child_data = container_of(child, &Data<Key,Value>::node_);
                data->key_ = child_data->key_;
                data->value_ = child_data->value_;
                root->left_ = child->left_;
                root->right_ = child->right_;
                if (root->left_) root->left_->parent_ = root;
                if (root->right_) root->right_->parent_ = root;
                delete child_data;
            }
        }else{
            auto succ = get_mini_node<Key,Value>(root->right_);
            auto succ_data = container_of(succ,&Data<Key,Value>::node_);
            data->key_ = succ_data->key_;
            data->value_ = succ_data->value_;
            root->right_ = deleteNode<Key,Value>(root->right_, succ_data->key_);
        }
    }

    root->height_ = 1 + std::max(get_height(root->left_), get_height(root->right_));
    int balance = getBalance(root);

    if(balance>1){
        auto left_data = root->left_ ? container_of(root->left_,&Data<Key,Value>::node_) : nullptr;
        if(left_data && getBalance(root->left_) >= 0){
            return rot_right(root);
        }else{
            root->left_ = rot_left(root->left_);
            return rot_right(root);
        }
    }
    if(balance<-1){
        auto right_data = root->right_ ? container_of(root->right_,&Data<Key,Value>::node_) : nullptr;
        if(right_data && getBalance(root->right_) <= 0){
            return rot_left(root);
        }else{
            root->right_ = rot_right(root->right_);
            return rot_left(root);
        }
    }
    return root;
}

template<typename Key,typename Value>
class AVLMap{
public:
    AVLMap();
    ~AVLMap();
    Value* get(const Key&);
    void put(const Key&,const Value&);
    void remove(const Key&);
    std::vector<std::pair<Key,Value>> inorderTraversal();
private:
    void inorderHelper(AVLNode* node,std::vector<std::pair<Key,Value>>& keys_arr);
private:
    AVLNode* root;
};

template<typename Key,typename Value>
AVLMap<Key,Value>::AVLMap():root(nullptr) {}

template<typename Key,typename Value>
AVLMap<Key,Value>::~AVLMap() {
    std::function<void(AVLNode*)> destroy=[&](AVLNode* node){
        if (node) {
            destroy(node->left_);
            destroy(node->right_);
            auto data = container_of(node, &Data<Key, Value>::node_);
            delete data;
        }
    };
    destroy(root);
}

template<typename Key,typename Value>
Value* AVLMap<Key,Value>::get(const Key& key){
    return search<Key,Value>(root,key);
}

template<typename Key,typename Value>
void AVLMap<Key,Value>::put(const Key& key,const Value& value) {
    root=insert<Key,Value>(root,key,value);
}

template<typename Key,typename Value>
void AVLMap<Key,Value>::remove(const Key& key){
    root=deleteNode<Key,Value>(root,key);
}

template<typename Key,typename Value>
std::vector<std::pair<Key,Value>> AVLMap<Key,Value>::inorderTraversal(){
    std::vector<std::pair<Key,Value>> res;
    inorderHelper(root,res);
    return res;
}

template<typename Key,typename Value>
void AVLMap<Key,Value>::inorderHelper(AVLNode* node,std::vector<std::pair<Key,Value>>& keys_arr){
    if(!node) return;
    inorderHelper(node->left_,keys_arr);
    auto data=container_of(node,&Data<Key,Value>::node_);
    keys_arr.emplace_back(data->key_,data->value_);
    inorderHelper(node->right_,keys_arr);
}

#endif //REDIS_AVLMAP_H
