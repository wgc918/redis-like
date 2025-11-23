#pragma once

#include <cstdint>
#include <cstddef>

// 哈希表节点
// 需要将节点嵌入到实际数据结构中
struct HNode
{
    HNode *next = nullptr;
    uint64_t hcode = 0;
};

// 哈希表
// 使用拉链法解决哈希冲突
class HTab
{
private:
    HNode **tab;   // 槽位数组，每个元素是指向链表的指针
    uint32_t mask; // 数组大小掩码，用于快速计算出槽位
    uint32_t size; // 当前存储的键数量

public:
    // 禁用拷贝构造和赋值
    HTab(const HTab &) = delete;
    HTab &operator=(const HTab &) = delete;

    // 允许移动语义
    HTab(HTab &&other) noexcept
        : tab(other.tab), mask(other.mask), size(other.size)
    {
        other.tab = nullptr;
        other.mask = 0;
        other.size = 0;
    }

    HTab &operator=(HTab &&other) noexcept
    {
        if (this != &other)
        {
            delete[] tab;
            tab = other.tab;
            mask = other.mask;
            size = other.size;
            other.tab = nullptr;
            other.mask = 0;
            other.size = 0;
        }
        return *this;
    }

    HTab();
    HTab(uint32_t size);
    ~HTab();

    void h_insert(HNode *node);
    HNode **h_lookup(HNode *key, bool (*eq)(HNode *, HNode *));
    HNode *h_detach(HNode **from);

    uint32_t get_size();
    uint32_t get_mask();
    HNode **data();
    void h_clean_up();

protected:
    uint64_t get_pos(uint64_t hcode);
};

// 使用两个哈希表实现渐进式重哈希
class HMap
{
private:
    HTab newTab;                           // 新哈希表
    HTab oldTab;                           // 旧哈希表
    uint32_t migrate_pos = 0;              // 当前迁移到的位置（在旧表中的索引）
    const uint32_t k_rehashing_work = 128; // 每次最多迁移的节点数
    const uint32_t k_max_load_factor = 8;  // 最大负载因子（平均每个槽位放8个）
    const uint32_t init_size = 8;          // 最开始的哈希表大小（8个槽位）
protected:
    // 帮助重哈希
    void hm_help_rehashing();

    // 触发重哈希
    void hm_trigger_rehashing();

public:
    // 禁用拷贝和赋值
    HMap(const HMap &) = delete;
    HMap &operator=(const HMap &) = delete;

    // 允许移动
    HMap(HMap &&) = default;
    HMap &operator=(HMap &&) = default;

    HMap();
    ~HMap();

    HNode *hm_lookup(HNode *key, bool (*eq)(HNode *, HNode *));
    void hm_insert(HNode *node);
    HNode *hm_delete(HNode *key, bool (*eq)(HNode *, HNode *));
    uint64_t hm_size();
    void hm_clean_up();
};