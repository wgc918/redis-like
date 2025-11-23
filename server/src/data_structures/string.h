#pragma once
#include "base.h"
#include <string>
#include <cstdint>
#include "./hashTable.h"

#define container_of(ptr, T, member) \
    reinterpret_cast<T *>(reinterpret_cast<char *>(ptr) - offsetof(T, member))

struct Entry_str
{
    std::string key;
    std::string value;
    HNode node;
};

class StringEntry
{
private:
    Entry_str entry;

public:
    // 构造函数
    StringEntry() = default;
    StringEntry(std::string key, std::string value = "");

    StringEntry(const StringEntry &) = delete;
    StringEntry &operator=(const StringEntry &) = delete;
    StringEntry(StringEntry &&) = default;
    StringEntry &operator=(StringEntry &&) = default;

    // 访问器
    const std::string &key() const { return entry.key; }
    const std::string &value() const { return entry.value; }
    std::string &value() { return entry.value; }
    HNode *node() { return &entry.node; }
    const HNode *node() const { return &entry.node; }

    // 业务操作
    std::string get(HMap &hmap);
    bool set(HMap &hmap);
    bool del(HMap &hmap);

    // 工具方法
    uint64_t hash();
};

// 哈希比较
bool equals(HNode *a, HNode *b);