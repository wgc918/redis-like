#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

// AVL树节点
// 需要将节点嵌入到实际数据结构中
struct AVLNode
{
    AVLNode *parent; // 父节点
    AVLNode *left;   // 左孩子
    AVLNode *right;  // 右孩子
    uint32_t height; // 树高
    uint32_t cnt;    // 子树节点数（含自己）

    AVLNode() : parent(nullptr), left(nullptr), right(nullptr), height(1), cnt(1) {}

    AVLNode(AVLNode *p, AVLNode *l, AVLNode *r, uint32_t h, uint32_t c)
    {
        parent = p;
        left = l;
        right = r;
        height = h;
        cnt = c;
    }
};

class AVLTree
{
private:
    AVLNode *root; // 根节点

protected:
    // 更新node节点的height和cnt
    void avl_update(AVLNode *node);

    // 将失衡节点左旋 RR型
    AVLNode *rot_left(AVLNode *node);

    // 将失衡节点右旋 LL型
    AVLNode *rot_right(AVLNode *node);

    // 先将失衡节点左孩子左旋然后再右旋失衡节点 LR型
    AVLNode *rot_left_right(AVLNode *node);

    // 先将失衡节点右孩子右旋然后再左旋失衡节点 RL型
    AVLNode *rot_right_left(AVLNode *node);

    // 综合修复平衡
    AVLNode *avl_fix(AVLNode *node);

    // 删除最多只有一个孩子的节点
    AVLNode *avl_del(AVLNode *node);

    // 返回该节点树高
    uint32_t avl_height(AVLNode *node);

    // 返回该节点的子树节点树
    uint32_t avl_cnt(AVLNode *node);

    // 析构时使用 后续遍历依次析构每一个节点
    void postorder(AVLNode *node);

    void inorder(AVLNode *node,std::vector<AVLNode*>&results);

    // 递归收集排名范围内的所有节点
    void
    collect_in_range(AVLNode *node, int &current_rank, int min_rank, int max_rank, std::vector<AVLNode *> &result);

public:
    AVLTree();
    ~AVLTree();

    // 查找
    AVLNode *avl_find(AVLNode *node, bool (*less)(AVLNode *node1, AVLNode *node2));

    // 判断树中是否有该节点，有则删除
    AVLNode *avl_delete(AVLNode *node, bool (*less)(AVLNode *node1, AVLNode *node2));

    // 可直接删除，搭配哈希表一起使用，可提升效率，通过哈希表判断该节点是否存在
    AVLNode *avl_delete_(AVLNode *node);

    // 搭配哈希表使用，直接插入
    void avl_insert(AVLNode *node, bool (*less)(AVLNode *node1, AVLNode *node2));

    AVLNode *avl_offset(AVLNode *node, int32_t offset);

    // 返回node节点在avl中的中序遍历名次
    int avl_rank(AVLNode *node);

    void avl_range_by_rank(int min_rank, int max_rank, std::vector<AVLNode *> &results);

    void avl_inorder(std::vector<AVLNode *> &results);

    void avl_clean_up();
};