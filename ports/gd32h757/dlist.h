#ifndef __LIST_H__
#define __LIST_H__

struct Dlist_node{
    struct Dlist_node * next;
    struct Dlist_node * prev;
};
typedef struct Dlist_node Dlist_node_t;

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))


/**
 * @brief 遍历链表
 * @param pos 结构体指针
 * @param n 临时结构体指针
 * @param head 链表头
 * @param type 结构体类型
 * @param member 结构体中作为链表节点的成员名
 */
#define list_for_each_entry_safe(pos, n, head, type, member)                \
    for (pos = container_of((head)->next, type, member),                      \
         n = container_of(pos->member.next, type, member);                    \
         &pos->member != (head);                                            \
         pos = n, n = container_of(n->member.next, type, member))


/* 初始化链表头 */
static inline void Dlist_init(Dlist_node_t *list)
{
    list->next = list;
    list->prev = list;
}

/* 插入新节点：在 head 后插入 new 节点 */
static inline void Dlist_insert_after(Dlist_node_t *head, Dlist_node_t *new_node)
{
    new_node->next = head->next;
    new_node->prev = head;
    head->next->prev = new_node;
    head->next = new_node;
}

/* 删除节点 */
static inline void Dlist_remove(Dlist_node_t *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->next = node;
    node->prev = node;
}

/* 判断链表是否为空 */
static inline int Dlist_is_empty(const Dlist_node_t *head)
{
    return head->next == head;
}

#endif //__LIST_H__
