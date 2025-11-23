#pragma once
#include "base.h"
#include <string>
#include <cstdint>
#include "./hashTable.h"
#include "avl.h"
#include <vector>
#include <utility>

#define container_of(ptr, T, member) \
    reinterpret_cast<T *>(reinterpret_cast<char *>(ptr) - offsetof(T, member))

// 将有序集的键值对数据抽象为一个结构体，成员存储在 hamp和tree中
struct Value
{
    HMap hmap;
    AVLTree tree;
};

// 定义哈希表中存储一个ZsetNode对象
struct ZsetNode
{
    std::string key; // 顶级哈希表中的关键字
    Value *value;
    HNode node;
};

// 管理顶级哈希表中的ZsetNode
class Zset
{
private:
    ZsetNode node_;

public:
    Zset(const std::string &key);
    // 在顶级哈希表中创建一个ZsetNode对象
    Value *create(HMap &hmap);

    // 顶级哈希表中是否存在
    Value *exsit(HMap &hmap);

    // 在顶级哈希表中删除一个ZsetNode对象(删除整个有序集合)
    bool zdel(HMap &hmap);

private:
    uint64_t hash();
};

// 有序集中具体的一个元素
struct Entry_zset
{
    double score;     // 分数
    std::string name; // 成员
    AVLNode avl_node;
    HNode hash_node;
};

// 管理一个集合中的所有元素，即对顶级哈希表中的一个有序集的值的管理
class ZsetEntry
{
private:
    Entry_zset entry_;

public:
    ZsetEntry(double score, const std::string &name);
    ZsetEntry(const std::string &name);

    // ZADD key score member：添加一个元素sorted set ，如果已经存在则更新其score值
    bool zadd(Value *val);

    // ZREM key member：删除sorted set中的一个指定元素
    bool zrem(Value *val);

    // ZSCORE key member : 获取sorted set中的指定元素的score值 isok为真时，表示member存在，返回数据可信，反之返回数据无效
    double zscore(Value *val, bool &isok);

    // ZRANK key member：获取sorted set 中的指定元素的排名
    int zrank(Value *val, bool &isok);

    // ZCARD key：获取sorted set中的元素个数
    int zcard(Value *val);

    // ZRANGE key min max：按照score排序后，获取指定排名范围内的元素
    std::vector<std::pair<std::string, double>> zrange(Value *val, int min_rank, int max_rank);

    //ZALL key :按照score排名后，返回集合所有元素
    std::vector<std::pair<std::string, double>> zall(Value *val);

protected:
    uint64_t hash();
};

// 哈希比较 用于顶级哈希表key比较
bool equals_(HNode *a, HNode *b);

// 哈希比较 用于集合内部哈希表key比较
bool equals_entry(HNode *a, HNode *b);

// avl树节点的比较
bool less(AVLNode *a, AVLNode *b);