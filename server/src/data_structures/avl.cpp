#include "avl.h"
#include <vector>

AVLTree::AVLTree()
{
    root = nullptr;
}

AVLTree::~AVLTree()
{
    // postorder(root);
}

/// @brief 查找树中是否存在该节点
/// @param node 查找节点
/// @param less 节点比较函数指针
/// @return 存在则返回该节点指针，否则返回nullptr
AVLNode *AVLTree::avl_find(AVLNode *node, bool (*less)(AVLNode *node1, AVLNode *node2))
{
    AVLNode *cur = root;
    while (cur)
    {
        if (less(node, cur))
            cur = cur->left;
        else if (less(cur, node))
            cur = cur->right;
        else
            return cur;
    }
    return nullptr;
}

AVLNode *AVLTree::avl_delete(AVLNode *node, bool (*less)(AVLNode *node1, AVLNode *node2))
{
    // 不存在直接退出
    if (!avl_find(node, less))
        return root;
    // 如果删除的节点，其孩子最多只有一个
    if (!node->left || !node->right)
        return avl_del(node);

    // 对于有两个孩子的节点，需要找到其中序遍历序列中相邻的节点来替换它
    // 这里我选择其后继节点，即右子树中最小的节点
    AVLNode *successor = node->right;
    while (successor->left)
        successor = successor->left;

    // 删除后继节点
    root = avl_del(successor);
    // 用后继节点替换node
    successor->parent = node->parent;
    successor->left = node->left;
    successor->right = node->right;
    if (successor->left)
        successor->left->parent = successor;
    if (successor->right)
        successor->right->parent = successor;
    avl_update(successor);
    // 将后继节点连接到node的父节点
    AVLNode **from = &root;
    AVLNode *parent = node->parent;
    if (parent)
        from = parent->left == node ? &parent->left : &parent->right;
    *from = successor;

    return root;
}

AVLNode *AVLTree::avl_delete_(AVLNode *node)
{
    // 如果删除的节点，其孩子最多只有一个
    if (!node->left || !node->right)
        return avl_del(node);

    // 对于有两个孩子的节点，需要找到其中序遍历序列中相邻的节点来替换它
    // 这里我选择其后继节点，即右子树中最小的节点
    AVLNode *successor = node->right;
    while (successor->left)
        successor = successor->left;

    // 删除后继节点
    root = avl_del(successor);
    // 用后继节点替换node
    successor->parent = node->parent;
    successor->left = node->left;
    successor->right = node->right;
    if (successor->left)
        successor->left->parent = successor;
    if (successor->right)
        successor->right->parent = successor;
    avl_update(successor);
    // 将后继节点连接到node的父节点
    AVLNode **from = &root;
    AVLNode *parent = node->parent;
    if (parent)
        from = parent->left == node ? &parent->left : &parent->right;
    *from = successor;

    // 清理被删除节点的指针，防止悬挂引用
    node->parent = nullptr;
    node->left = nullptr;
    node->right = nullptr;
    node->height = 1;
    node->height = 1;

    return root;
}

void AVLTree::avl_insert(AVLNode *node, bool (*less)(AVLNode *node1, AVLNode *node2))
{
    AVLNode **from = &root;
    AVLNode *cur = nullptr;
    while (*from)
    {
        cur = *from;
        from = less(node, cur) ? &cur->left : &cur->right;
    }
    *from = node;
    node->parent = cur;
    root = avl_fix(node);
}

/// @brief 根据相对偏移量在AVL树中查找节点
/// @param node 起始节点
/// @param offset 相对偏移量(中序遍历序列的排名差)
/// @return 距离起始节点偏移量为offset的节点，如果超出范围返回NULL
AVLNode *AVLTree::avl_offset(AVLNode *node, int32_t offset)
{
    int pos = 0; // 当前位置相对于起始节点的排名差异
    while (offset != pos)
    {
        // 目标节点在当前节点右子树
        if (pos < offset && pos + avl_cnt(node->right) >= offset)
        {
            node = node->right;
            // 当前节点左子树的节点数 + 当前节点自身
            pos += avl_cnt(node->left) + 1;
        }
        // 目标节点在当前节点左子树
        else if (pos > offset && pos - avl_cnt(node->left) <= offset)
        {
            node = node->left;
            pos -= avl_cnt(node->right) + 1;
        }
        else // 目标不在当前节点的直接子树中，需要向父节点回溯
        {
            AVLNode *parent = node->parent;
            if (!parent)
                return nullptr;

            if (parent->left == node)
                pos += avl_cnt(node->right) + 1;
            else
                pos -= avl_cnt(node->left) + 1;
            node = parent;
        }
    }
    return node;
}

int AVLTree::avl_rank(AVLNode *node)
{
    if (!node)
        return 0;

    int rank = avl_cnt(node->left) + 1; // 左子树节点数 + 当前节点

    // 向上遍历父节点，累加所有比当前节点小的节点
    AVLNode *current = node;
    while (current->parent)
    {
        if (current == current->parent->right)
        {
            // 如果是右孩子，父节点和父节点的左子树都比当前节点小
            rank += avl_cnt(current->parent->left) + 1;
        }
        current = current->parent;
    }

    return rank;
}

/// @brief 获取排名在[min_rank,max_rank]中的所有元素
/// @param min_rank
/// @param max_rank
/// @param results 结果集数组
void AVLTree::avl_range_by_rank(int min_rank, int max_rank, std::vector<AVLNode *> &results)
{
    if (min_rank > max_rank || !root)
        return;
    int curr_rank = 1;
    collect_in_range(root, curr_rank, min_rank, max_rank, results);
}

void AVLTree::avl_inorder(std::vector<AVLNode *> &results)
{
    inorder(root, results);
}

void AVLTree::avl_clean_up()
{
    root = nullptr;
}

/// @brief 更新一个节点的高度和子树节点数为子树最大值+1
/// @param node
void AVLTree::avl_update(AVLNode *node)
{
    auto max = [](uint32_t l_cnt, uint32_t r_cnt)
    { return l_cnt > r_cnt ? l_cnt : r_cnt; };
    node->height = 1 + max(avl_height(node->left), avl_height(node->right));
    node->cnt = 1 + avl_cnt(node->left) + avl_cnt(node->right);
}

/// @brief RR型，失衡节点的平衡因子为-2，将失衡节点左旋，失衡节点作为失衡节点右孩子的左孩子，失衡节点的右孩子作为新的根节点
/// @param node 失衡节点
/// @return 返回原以node为根节点的子树的新的根节点
AVLNode *AVLTree::rot_left(AVLNode *node)
{
    AVLNode *parent = node->parent; // 保存node节点的父节点，以便将变换后的子树与整颗树连接
    AVLNode *new_root = node->right;

    // 如果失衡节点的右孩子有左孩子，则将它作为node的右孩子
    AVLNode *the_left_child_in_conflict = new_root->left;
    node->right = the_left_child_in_conflict;
    if (the_left_child_in_conflict)
        the_left_child_in_conflict->parent = node;
    node->parent = new_root;
    new_root->parent = parent;
    new_root->left = node;

    // 更新节点的height和cnt
    avl_update(node);
    avl_update(new_root);
    return new_root;
}

/// @brief LL型，失衡节点的平衡因子为2，将失衡节点右旋，失衡节点作为失衡节点左孩子的右孩子，失衡节点的左孩子作为新的根节点
/// @param node 失衡节点
/// @return 返回原以失衡节点为根的子树的新的根节点
AVLNode *AVLTree::rot_right(AVLNode *node)
{
    AVLNode *parent = node->parent; // 保存node节点的父节点，以便将变换后的子树与整颗树连接
    AVLNode *new_root = node->left;

    // 冲突的右孩变左孩
    AVLNode *the_right_child_in_conflict = new_root->right;
    node->left = the_right_child_in_conflict;
    if (the_right_child_in_conflict)
        the_right_child_in_conflict->parent = node;
    node->parent = new_root;
    new_root->parent = parent;
    new_root->right = node;

    // 更新节点数据
    avl_update(node);
    avl_update(new_root);
    return new_root;
}

/// @brief LR型，失衡节点的平衡因子为2，失衡节点的左孩子的平衡因子为-1，先左旋左孩子，然后右旋失衡节点，新的根节点为失衡节点的左孩子的右孩子
/// @param node 失衡节点
/// @return 返回原以失衡节点为根的子树的新的根节点
AVLNode *AVLTree::rot_left_right(AVLNode *node)
{
    if (avl_height(node->left->left) <= avl_height(node->left->right))
        node->left = rot_left(node->left);
    return rot_right(node);
}

/// @brief RL型，失衡节点的平衡因子为-2，失衡节点的右孩子的平衡因子为1，先右旋右孩子，然后左旋失衡节点，新的根节点为失衡节点右孩子的左孩子
/// @param node 失衡节点
/// @return 返回原以失衡节点为根的子树的新的根节点
AVLNode *AVLTree::rot_right_left(AVLNode *node)
{
    if (avl_height(node->right->left) >= avl_height(node->right->right))
        node->right = rot_right(node->right);
    return rot_left(node);
}

/// @brief 从当前节点开始，判断节点是否失衡，失衡则进行相应的旋转，直到根节点
/// @param node
/// @return 返回经平衡调整后的整颗树的根节点
AVLNode *AVLTree::avl_fix(AVLNode *node)
{
    while (true)
    {
        AVLNode *parent = node->parent;
        AVLNode **from = &node;
        // 如果当前节点不是根节点，需要记录当前节点是其父节点的左孩或右孩，以便将旋转后的子树与其连接
        if (parent)
            from = parent->left == node ? &parent->left : &parent->right;

        avl_update(node);

        uint32_t Lh = avl_height(node->left);
        uint32_t Rh = avl_height(node->right);

        // LL型或LR型
        if (Lh == Rh + 2)
            *from = rot_left_right(node); // 将node的父节点的左孩子或右孩子设置为旋转后的子树的新的根节点
        else if (Rh == Lh + 2)
            *from = rot_right_left(node);
        // 如果当前节点已经是根节点，则直接返回新的根节点
        if (!parent)
            return *from;
        node = parent;
    }
}

/// @brief 删除最多只有一个孩子的节点
/// @param node
/// @return 返回删除了该节点，然后经过平衡调整后的整颗树的根节点
AVLNode *AVLTree::avl_del(AVLNode *node)
{
    if (!node)
        return root;
    AVLNode *only_child = node->left ? node->left : node->right;
    AVLNode *parent = node->parent;

    if (only_child)
        only_child->parent = parent;

    // 如果删除的是根节点,直接将其孩子作为新的根节点
    if (!parent)
    {
        node->parent = nullptr;
        node->left = nullptr;
        node->right = nullptr;
        node->height = 1;
        node->height = 1;

        root = only_child;

        return root;
    }
    // 如果删除的不是根节点，则将其孩子顶替自己去作为父亲的左孩子或右孩子，同时可能会导致父节点失衡，所以需要调整
    AVLNode **from = parent->left == node ? &parent->left : &parent->right;
    *from = only_child;

    root = avl_fix(parent);

    // 清理被删除节点的指针，防止悬挂引用
    node->parent = nullptr;
    node->left = nullptr;
    node->right = nullptr;
    node->height = 1;
    node->height = 1;

    return root;
}

uint32_t AVLTree::avl_height(AVLNode *node)
{
    return node ? node->height : 0;
}

uint32_t AVLTree::avl_cnt(AVLNode *node)
{
    return node ? node->cnt : 0;
}

void AVLTree::postorder(AVLNode *node)
{
    if (!node)
        return;
    postorder(node->left);
    postorder(node->right);
    delete node;
}

void AVLTree::inorder(AVLNode *node, std::vector<AVLNode *> &results)
{
    if (!node)
        return;
    inorder(node->left, results);
    results.push_back(node);
    inorder(node->right, results);
}

void AVLTree::collect_in_range(AVLNode *node, int &current_rank, int min_rank, int max_rank, std::vector<AVLNode *> &result)
{
    if (!node || current_rank > max_rank)
        return;

    // 遍历左子树
    if (node->left)
    {
        collect_in_range(node->left, current_rank, min_rank, max_rank, result);
    }

    // 处理当前节点
    if (current_rank >= min_rank && current_rank <= max_rank)
    {
        result.push_back(node);
    }
    current_rank++;

    // 遍历右子树
    if (current_rank <= max_rank && node->right)
    {
        collect_in_range(node->right, current_rank, min_rank, max_rank, result);
    }
}
