/*******************************************************************************
* Company		  :  unicom-IOT
* Author		  :  author
* Version		  :  V1.0
* Date			  :  2021-05-27
* Description	  :  util_list.h
********************************************************************************/

/********************************* Include File ********************************/



/********************************* Macro Definition ****************************/


#define aos_offsetof(type, member)   ((size_t)&(((type *)0)->member))

#define aos_container_of(ptr, type, member) \
		((type *) ((char *) (ptr) - aos_offsetof(type, member)))


#define dlist_for_each_entry(queue, node, type, member) \
    for (node = aos_container_of((queue)->next, type, member); \
         &node->member != (queue); \
         node = aos_container_of(node->member.next, type, member))
#define list_entry(ptr, type, member) \
    aos_container_of(ptr, type, member)


#define list_for_each_entry_safe(pos, n, head, member, type)         \
for (pos = list_entry((head)->next, type, member), \
     n = list_entry(pos->member.next, type, member);    \
     &pos->member != (head);                    \
     pos = n, n = list_entry(n->member.next, type, member))
#define list_for_each_entry(pos, head, member, type)             \
		dlist_for_each_entry(head, pos, type, member)


#define dlist_head                       doublist



/********************************* Type Definition *****************************/

/*double link list */


typedef  struct doublist {
    struct doublist *prev;
    struct doublist *next;
}  doublist_s;


static inline void DList_HEAD_Init(doublist_s *list)
{
    list->next = list;
    list->prev = list;
}

static inline void dlist_del(doublist_s *node)
{
    doublist_s *prev = node->prev;
    doublist_s *next = node->next;

    prev->next = next;
    next->prev = prev;
}
static inline void __dlist_add(doublist_s *node, doublist_s *prev, doublist_s *next)
{
    node->next = next;
    node->prev = prev;

    prev->next = node;
    next->prev = node;
}

static inline void dlist_add_tail(doublist_s *node, doublist_s *queue)
{
    __dlist_add(node, queue->prev, queue);
}
static inline int __dlist_entry_number(doublist_s *queue)
{
    int num;
    doublist_s *cur = queue;
    for (num = 0; cur->next != queue; cur = cur->next, num++)
        ;

    return num;
}


#define dlist_entry_number(head) \
    __dlist_entry_number(head)


/********************************* Variables ***********************************/

/********************************* Function ************************************/





