#include "zset.h"
#include <iostream>

Zset::Zset(const std::string &key)
{
    node_.key = key;
    node_.node.hcode = hash();
    node_.value = nullptr;
}

/// @brief 在顶级哈希表中插入一个ZsetNode
/// @param hmap 顶级哈希表
/// @return 插入成功则返回Value指针，否则返回空指针
Value *Zset::create(HMap &hmap)
{
    Value *ret = exsit(hmap);
    if (ret)
        return ret;
    try
    {
        ZsetNode *insert_node = new ZsetNode();
        insert_node->key = node_.key;
        insert_node->node.hcode = node_.node.hcode;
        insert_node->value = new Value();
        hmap.hm_insert(&insert_node->node);
        ret = insert_node->value;
    }
    catch (std::exception &e)
    {
        return nullptr;
    }
    return ret;
}

Value *Zset::exsit(HMap &hmap)
{
    HNode *target = hmap.hm_lookup(&node_.node, equals_);
    if (target)
    {
        return container_of(target, ZsetNode, node)->value;
    }
    return nullptr;
}

bool Zset::zdel(HMap &hmap)
{
    try
    {
        // 首先在顶级哈希表中找到集合的节点
        HNode *target = hmap.hm_delete(&node_.node, equals_);
        if (!target)
            return true;
        ZsetNode *p = container_of(target, ZsetNode, node);

        // 通过遍历avl树收集集合中所有元素
        std::vector<AVLNode *> all_elements;
        p->value->tree.avl_inorder(all_elements);

        for (auto &node : all_elements)
        {
            Entry_zset *cur_ds = container_of(node, Entry_zset, avl_node);
            // 清理哈希表钩子
            p->value->hmap.hm_delete(&cur_ds->hash_node, equals_entry);
            // 清理avl树钩子
            p->value->tree.avl_delete_(node);
            // 释放实际数据结构内存
            delete cur_ds;
            cur_ds = nullptr;
        }
        // 清理完集合中所有元素后，释放顶级哈希表中该集合的节点。
        p->value->tree.avl_clean_up();
        p->value->hmap.hm_clean_up();
        delete p->value;
        p->value = nullptr;
        delete p;
        p = nullptr;
    }
    catch (const std::exception &e)
    {
        return false;
    }
    return true;
}

uint64_t Zset::hash()
{
    uint32_t h = 0x811C9DC5;
    for (char &ch : node_.key)
    {
        h = (h + ch) * 0x01000193;
    }
    return h;
}

bool equals_(HNode *a, HNode *b)
{
    ZsetNode *a_ = container_of(a, ZsetNode, node);
    ZsetNode *b_ = container_of(b, ZsetNode, node);
    return a_->key == b_->key;
}

bool equals_entry(HNode *a, HNode *b)
{
    Entry_zset *a_ = container_of(a, Entry_zset, hash_node);
    Entry_zset *b_ = container_of(b, Entry_zset, hash_node);
    return a_->name == b_->name;
}

ZsetEntry::ZsetEntry(double score, const std::string &name)
{
    entry_.name = name;
    entry_.score = score;
    entry_.hash_node.hcode = hash();
}

ZsetEntry::ZsetEntry(const std::string &name)
{
    entry_.name = name;
    entry_.score = 0;
    entry_.hash_node.hcode = hash();
}

bool ZsetEntry::zadd(Value *val)
{
    try
    {
        // 先通过哈希表判断，该元素是否存在
        HNode *target = val->hmap.hm_lookup(&entry_.hash_node, equals_entry);
        if (target)
        {
            // 先将分数为旧值的节点从avl中删除，将分数更新后，再将节点插入到avl中，以达到分数更新后，排名也更新的效果
            Entry_zset *p = container_of(target, Entry_zset, hash_node);
            val->tree.avl_delete_(&p->avl_node);
            // 更新分数
            p->score = entry_.score;
            // 重新插入以调整avl树
            val->tree.avl_insert(&p->avl_node, less);
        }
        else
        {
            // 若不存在则开辟空间创建一个新Entry_zset，并分别插入到哈希表、avl中
            Entry_zset *insert_entry = new Entry_zset();
            insert_entry->name = entry_.name;
            insert_entry->score = entry_.score;
            insert_entry->hash_node.hcode = hash();

            val->tree.avl_insert(&insert_entry->avl_node, less);
            val->hmap.hm_insert(&insert_entry->hash_node);
        }
    }
    catch (std::exception &e)
    {
        return false;
    }
    return true;
}

bool ZsetEntry::zrem(Value *val)
{
    try
    { // 通过哈希表判断要删除的元素是否存在
        HNode *target = val->hmap.hm_lookup(&entry_.hash_node, equals_entry);
        if (!target)
            return true;
        Entry_zset *p = container_of(target, Entry_zset, hash_node);
        val->hmap.hm_delete(&entry_.hash_node, equals_entry);
        val->tree.avl_delete_(&p->avl_node);

        delete p;
    }
    catch (const std::exception &e)
    {
        return false;
    }
    return true;
}

double ZsetEntry::zscore(Value *val, bool &isok)
{
    HNode *target = val->hmap.hm_lookup(&entry_.hash_node, equals_entry);
    if (!target)
    {
        isok = false;
        return 0.0;
    }
    isok = true;
    return container_of(target, Entry_zset, hash_node)->score;
}

int ZsetEntry::zrank(Value *val, bool &isok)
{
    HNode *target = val->hmap.hm_lookup(&entry_.hash_node, equals_entry);
    if (!target)
    {
        isok = false;
        return -1;
    }
    isok = true;
    return val->tree.avl_rank(&container_of(target, Entry_zset, hash_node)->avl_node);
}

int ZsetEntry::zcard(Value *val)
{
    return (int)val->hmap.hm_size();
}

std::vector<std::pair<std::string, double>> ZsetEntry::zrange(Value *val, int min_rank, int max_rank)
{
    std::vector<std::pair<std::string, double>> res; // 真实数据的结果集
    std::vector<AVLNode *> results;                  // 范围内的avl节点结果集
    val->tree.avl_range_by_rank(min_rank, max_rank, results);
    for (auto &item : results)
    {
        Entry_zset *p = container_of(item, Entry_zset, avl_node);
        res.emplace_back(p->name, p->score);
        // std::cout << p->name << " " << p->score << std::endl;
    }
    return res;
}

std::vector<std::pair<std::string, double>> ZsetEntry::zall(Value *val)
{
    std::vector<std::pair<std::string, double>> res;
    std::vector<AVLNode *> results;
    val->tree.avl_inorder(results);
    for (auto &item : results)
    {
        Entry_zset *p = container_of(item, Entry_zset, avl_node);
        res.emplace_back(p->name, p->score);
        // std::cout << p->name << " " << p->score << std::endl;
    }
    return res;
}

uint64_t ZsetEntry::hash()
{
    uint32_t h = 0x811C9DC5;
    for (char &ch : entry_.name)
    {
        h = (h + ch) * 0x01000193;
    }
    return h;
}

bool less(AVLNode *a, AVLNode *b)
{
    Entry_zset *a_ = container_of(a, Entry_zset, avl_node);
    Entry_zset *b_ = container_of(b, Entry_zset, avl_node);

    if (a_->score == b_->score)
        return a_->name < b_->name;
    else
        return a_->score < b_->score;
}