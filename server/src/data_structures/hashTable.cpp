#include "hashTable.h"
#include "../utils/logger/logger.h"

HTab::HTab()
{
    this->tab = nullptr;
    this->mask = 0;
    this->size = 0;
}

HTab::HTab(uint32_t n)
{
    // 待优化，不是2的幂时，不能创建对象
    if (n == 0 || ((n - 1) & n) != 0)
    {
        Logger::error("HTab() 初始化哈希表时，大小不是2的幂");
        throw std::invalid_argument("哈希表大小必须是2的幂");
    }
    this->tab = new HNode *[n]();
    this->mask = n - 1;
    this->size = 0;
}

HTab::~HTab()
{
    // for (uint32_t i = 0; i <= mask; i++)
    // {
    //     HNode *curr = tab[i];
    //     while (curr != nullptr)
    //     {
    //         HNode *next = curr->next;
    //         delete curr; // 删除单个节点
    //         curr = next;
    //     }
    // }
    delete[] tab; // 删除指针数组
    tab = nullptr;
}

// 在对应槽位插入一个哈希节点
// 使用头插法
void HTab::h_insert(HNode *node)
{
    if (!node)
    {
        Logger::debug("h_insert() 插入失败：节点指针为空");
        return;
    }
    uint64_t pos = get_pos(node->hcode);
    HNode *next = tab[pos];
    node->next = next;
    tab[pos] = node;
    size++;
}

HNode **HTab::h_lookup(HNode *key, bool (*eq)(HNode *, HNode *))
{
    if (tab == nullptr)
    {
        Logger::debug("h_lookup() 哈希表为空，查找失败。");
        return nullptr;
    }
    uint64_t pos = get_pos(key->hcode);
    HNode **from = &tab[pos];
    for (HNode *cur = *from; cur != nullptr; from = &cur->next, cur = *from)
    {
        if (cur->hcode == key->hcode && eq(cur, key))
        {
            return from;
        }
    }
    return nullptr;
}

// 删除某一槽位的一个节点
HNode *HTab::h_detach(HNode **from)
{
    if (!from)
        return nullptr;
    HNode *node = *from;
    *from = node->next;
    size--;
    node->next = nullptr;
    return node;
}

uint32_t HTab::get_size()
{
    return size;
}

uint32_t HTab::get_mask()
{
    return mask;
}

HNode **HTab::data()
{
    return tab;
}

void HTab::h_clean_up()
{
    if (tab)
    {
        tab = nullptr;
        mask = 0;
        size = 0;
    }
}

// 除留余数法
uint64_t HTab::get_pos(uint64_t hcode)
{
    return hcode & mask;
}

void HMap::hm_help_rehashing()
{
    uint64_t nwork = 0; // 迁移的键数
    while (nwork < k_rehashing_work && oldTab.get_size() > 0)
    {
        // 查找非空槽位
        // 从槽位的第一个键开始迁移
        HNode **from = &oldTab.data()[migrate_pos];
        if (!*from)
        {
            migrate_pos++;
            continue;
        }
        newTab.h_insert(oldTab.h_detach(from));
        nwork++;
    }
    if (oldTab.get_size() == 0 && oldTab.data())
    {
        oldTab = HTab();
    }
}

void HMap::hm_trigger_rehashing()
{
    // 确保没有正在进行的重哈希
    if (oldTab.data() != nullptr)
        return;
    oldTab = std::move(newTab);
    newTab = HTab((oldTab.get_mask() + 1) << 1); // 新哈希表翻倍
    migrate_pos = 0;                             // 从头开始新一轮的重哈希
}

HMap::HMap() : oldTab()
{
    newTab = HTab(init_size);
}

HMap::~HMap()
{
}

HNode *HMap::hm_lookup(HNode *key, bool (*eq)(HNode *, HNode *))
{
    // 每次查询迁移指定键数
    hm_help_rehashing();
    // 先在新表中查询
    HNode **from = newTab.h_lookup(key, eq);
    if (!from)
    { // 查询旧表
        from = oldTab.h_lookup(key, eq);
    }
    return from ? *from : nullptr;
}

void HMap::hm_insert(HNode *node)
{
    newTab.h_insert(node);
    // 判断是否需要重哈希
    if (!oldTab.data())
    {
        uint64_t key_max = (newTab.get_mask() + 1) * k_max_load_factor;
        // 装填过载时触发重哈希
        if (newTab.get_size() >= key_max)
            hm_trigger_rehashing();
    }
    // 每次插入时迁移指定键数
    hm_help_rehashing();
}

HNode *HMap::hm_delete(HNode *key, bool (*eq)(HNode *, HNode *))
{
    // 每次删除时迁移指定键数
    hm_help_rehashing();
    // 先尝试在新表中删除
    HNode **from = newTab.h_lookup(key, eq);
    if (from)
        return newTab.h_detach(from);
    // 然后尝试在旧表中删除
    from = oldTab.h_lookup(key, eq);
    if (from)
        return oldTab.h_detach(from);
    return nullptr;
}

uint64_t HMap::hm_size()
{
    return newTab.get_size() + oldTab.get_size();
}

void HMap::hm_clean_up()
{
    if (newTab.data())
    {
        newTab.h_clean_up();
    }
    if (oldTab.data())
    {
        oldTab.h_clean_up();
    }
    migrate_pos = 0;
}
