/**
 * @file list.h
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-07-01
 */


#ifndef L_LIST_H
#define L_LIST_H

//LINK List

struct list_head{
    struct list_head *prev, *next;
};

#define LIST_HEAD_INIT(name) { &(name), &(name)}
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name);
#define INIT_LIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)




static inline int list_empty(const struct list_head *head)
{
		return head->next == head;
}
static inline void __list_add(struct list_head *new_,
        struct list_head *before,
        struct list_head *after)
{
    before->next = new_;
    new_->next = after;
    after->prev = new_;
    new_->prev = before;
}

#define list_add(new_, head) __list_add((new_), (head), ((head)->next))
#define list_add_tail(new_, head) __list_add(new_, (head)->prev, head)
static inline void list_del(struct list_head *entry)
{
    struct list_head *prev = entry->prev,
                     *next = entry->next;
    prev->next = next;
    next->prev = prev;
    entry->prev = NULL;
    entry->next = NULL;
}
#define list_entry(ptr, type, member) container_of(ptr, type, member)
#define container_of(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (char *)__mptr - offsetof_(type,member) );})
#define offsetof_(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define list_for_each(pos, head) \
	for (pos = (head)->next ; pos != (head); pos = pos->next)

#endif
