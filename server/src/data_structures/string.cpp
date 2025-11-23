#include "string.h"
#include "../utils/logger/logger.h"
#include <iostream>

StringEntry::StringEntry(std::string key, std::string value)
{
    entry.key = key;
    entry.value = value;
    entry.node.hcode = hash();
}

std::string StringEntry::get(HMap &hmap)
{
    // Logger::debug("=== 开始查找 ===");
    // Logger::debug("查找键: " + entry.key);
    HNode *node = hmap.hm_lookup(&entry.node, &equals);
    if (!node)
    {
        Logger::debug("查找失败");
        return "";
    }
    // Logger::debug("*** 查找成功 - 找到节点");

    Entry_str *foundEntry = container_of(node, Entry_str, node);

    // 详细输出找到的内容
    // Logger::debug("找到的Entry信息:");
    // Logger::debug("  - 地址: " + std::to_string((uintptr_t)foundEntry));
    // Logger::debug("  - 键: '" + foundEntry->key + "'");
    // Logger::debug("  - 值: '" + foundEntry->value + "'");
    // Logger::debug("  - 值长度: " + std::to_string(foundEntry->value.length()));
    // Logger::debug("  - 值空检查: " + std::to_string(foundEntry->value.empty()));

    std::string val = foundEntry->value;

    // Logger::debug("准备返回值: '" + val + "'");
    // std::cout << "控制台输出: " << val << std::endl;
    // Logger::debug("=== 查找结束 ===");
    return val;
}

bool StringEntry::set(HMap &hmap)
{
    try
    {
        // Logger::debug("=== 开始插入 ===");
        // Logger::debug("插入键: '" + entry.key + "'");
        // Logger::debug("插入值: '" + entry.value + "'");
        // Logger::debug("值长度: " + std::to_string(entry.value.length()));
        HNode *node = hmap.hm_lookup(&entry.node, &equals);
        // 如果已存在则更新
        if (node)
        {
            container_of(node, Entry_str, node)->value.swap(entry.value);
        }
        else
        { // 不存在则直接插入
            Entry_str *insert_entry = new Entry_str();
            insert_entry->key = entry.key;
            insert_entry->value = entry.value;
            insert_entry->node.hcode = entry.node.hcode;
            hmap.hm_insert(&insert_entry->node);
        }
    }
    catch (const std::exception &e)
    {
        return false;
    }
    return true;
}

bool StringEntry::del(HMap &hmap)
{
    HNode *node = hmap.hm_delete(&entry.node, &equals);
    if (!node)
    {
        return false;
    }
    delete container_of(node, Entry_str, node);
    return true;
}

// FNV
uint64_t StringEntry::hash()
{
    uint32_t h = 0x811C9DC5;
    for (char &ch : entry.key)
    {
        h = (h + ch) * 0x01000193;
    }
    return h;
}

bool equals(HNode *a, HNode *b)
{
    Entry_str *a_ = container_of(a, Entry_str, node);
    Entry_str *b_ = container_of(b, Entry_str, node);
    return a_->key == b_->key;
}