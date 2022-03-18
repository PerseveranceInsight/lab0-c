#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

#ifdef __Q_DEBUG__
#define FUNC_ENTRANCE printf("%s enters\n", __func__)
#define FUNC_EXIT printf("%s leaves\n", __func__)
#else
#define FUNC_ENTRANCE \
    do {              \
    } while (0)
#define FUNC_EXIT \
    do {          \
    } while (0)
#endif

#define check_null_exit(ptr, EXIT_LABEL, ERROR_LOG, ...) \
    {                                                    \
        if (!ptr) {                                      \
            printf(ERROR_LOG, ##__VA_ARGS__);            \
            printf("\n");                                \
            goto EXIT_LABEL;                             \
        }                                                \
    }

#define check_true_exit(FLAG, EXIT_LABEL, ERROR_LOG, ...) \
    {                                                     \
        if (FLAG) {                                       \
            printf(ERROR_LOG, ##__VA_ARGS__);             \
            printf("\n");                                 \
            goto EXIT_LABEL;                              \
        }                                                 \
    }

#define Q_MERGE_STRCMP_LIST(NODE, HEAD_NODE, NEW_HEAD) \
    {                                                  \
        NODE = NODE->next;                             \
        HEAD_NODE = NODE->prev;                        \
        list_del_init(HEAD_NODE);                      \
        if (!NEW_HEAD) {                               \
            NEW_HEAD = HEAD_NODE;                      \
        } else {                                       \
            list_add_tail(HEAD_NODE, NEW_HEAD);        \
        }                                              \
        if (HEAD_NODE == NODE)                         \
            NODE = NULL;                               \
    }

#define Q_MERGE_LIST(NODE, HEAD_NODE, NEW_HEAD) \
    {                                           \
        NODE = NODE->next;                      \
        HEAD_NODE = NODE->prev;                 \
        list_del_init(HEAD_NODE);               \
        list_add_tail(HEAD_NODE, NEW_HEAD);     \
        if (HEAD_NODE == NODE)                  \
            NODE = NULL;                        \
    }


/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *p_tmp_head = malloc(sizeof(struct list_head));
    FUNC_ENTRANCE;
    check_null_exit(p_tmp_head, EXIT_Q_NEW,
                    "%s occurs errror due to allocate p_tmp_head failed",
                    __func__);
    INIT_LIST_HEAD(p_tmp_head);
EXIT_Q_NEW:
    FUNC_EXIT;
    return p_tmp_head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    FUNC_ENTRANCE;
    struct list_head *p_tmp = NULL;
    element_t *p_ele = NULL;
    check_null_exit(l, EXIT_Q_FREE, "%s occurs error due to l is NULL",
                    __func__);
    if (!list_empty(l)) {
        for (p_tmp = l->next; p_tmp != l;) {
            p_ele = list_entry(p_tmp, element_t, list);
            p_tmp = p_tmp->next;
            free(p_ele->value);
            free(p_ele);
        }
    }
    free(l);
EXIT_Q_FREE:
    FUNC_EXIT;
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to bevalue_len.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    FUNC_ENTRANCE;
    bool retval = false;
    element_t *p_ele = NULL;
    check_null_exit(head, EXIT_Q_INSERT_HEAD,
                    "%s occurs error due to head is NULL", __func__);
    p_ele = malloc(sizeof(element_t));
    check_null_exit(p_ele, EXIT_Q_INSERT_HEAD,
                    "%s occurs error due to allocate p_ele failed", __func__);
    p_ele->value = strdup(s);
    check_null_exit(p_ele->value, EXIT_Q_INSERT_HEAD,
                    "%s occurs error due to p_ele->value is NULL", __func__);
    list_add(&p_ele->list, head);
    retval = true;
EXIT_Q_INSERT_HEAD:
    if ((p_ele) && (!retval))
        free(p_ele);
    FUNC_EXIT;
    return retval;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must ex:splitplicitly allocate space and copy the string into
 * it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    FUNC_ENTRANCE;
    bool retval = false;
    element_t *p_ele = NULL;
    check_null_exit(head, EXIT_Q_INSERT_TAIL,
                    "%s occurs error due to head is NULL", __func__);
    check_null_exit(s, EXIT_Q_INSERT_TAIL, "%s occurs error due to s is NULL",
                    __func__);
    p_ele = malloc(sizeof(element_t));
    check_null_exit(p_ele, EXIT_Q_INSERT_TAIL,
                    "%s occurs error due to allocate p_ele failed", __func__);
    p_ele->value = strdup(s);
    check_null_exit(p_ele->value, EXIT_Q_INSERT_TAIL,
                    "%s occurs error due to p_ele->value is NULL", __func__);
    list_add_tail(&p_ele->list, head);
    retval = true;
EXIT_Q_INSERT_TAIL:
    if ((p_ele) && (!retval))
        free(p_ele);
    FUNC_EXIT;
    return retval;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    FUNC_ENTRANCE;
    element_t *p_ele = NULL;
    uint32_t value_len = 0;
    check_null_exit(head, EXIT_Q_REMOVE_HEAD,
                    "%s occurs error due to head is NULL", __func__);
    if (list_empty(head))
        goto EXIT_Q_REMOVE_HEAD;
    p_ele = list_first_entry(head, element_t, list);
    list_del(head->next);
    if (sp) {
        value_len = strlen(p_ele->value);
        value_len = (value_len > bufsize) ? (bufsize - 1) : value_len;
        memset(sp, 0, bufsize);
        strncpy(sp, p_ele->value, value_len);
    }
EXIT_Q_REMOVE_HEAD:
    FUNC_EXIT;
    return p_ele;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    FUNC_ENTRANCE;
    element_t *p_ele = NULL;
    uint32_t value_len = 0;
    check_null_exit(head, EXIT_Q_REMOVE_TAIL,
                    "%s occurs error due to head is NULL", __func__);
    if (list_empty(head))
        goto EXIT_Q_REMOVE_TAIL;
    p_ele = list_last_entry(head, element_t, list);
    list_del(head->prev);
    if (sp) {
        value_len = strlen(p_ele->value);
        value_len = (value_len > bufsize) ? (bufsize - 1) : value_len;
        memset(sp, 0, bufsize);
        strncpy(sp, p_ele->value, value_len);
    }
EXIT_Q_REMOVE_TAIL:
    FUNC_EXIT;
    return p_ele;
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    int32_t retval = 0;
    struct list_head *p_node = NULL;
    check_null_exit(head, EXIT_Q_SIZE, "%s directly return", __func__);
    list_for_each (p_node, head)
        retval++;
EXIT_Q_SIZE:
    return retval;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    bool retval = false;
    struct list_head *p_del = NULL, *p_nn = NULL;
    element_t *p_ele = NULL;
    check_null_exit(head, EXIT_Q_DELETE_MID,
                    "%s occurs error due to head is NULL", __func__);
    p_del = head->next;
    p_nn = head->next->next;
    while ((p_nn != head) && (p_nn != head->prev)) {
        p_del = p_del->next;
        p_nn = p_nn->next->next;
    }
    p_ele = list_entry(p_del, element_t, list);
    list_del(p_del);
    q_release_element(p_ele);
    retval = true;
EXIT_Q_DELETE_MID:
    return retval;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    bool retval = false;
    bool deleted = false;
    struct list_head *p_cur = NULL, *p_next = NULL, *p_nn = NULL;
    check_null_exit(head, EXIT_Q_DELETE_DUP,
                    "%s occurs eror due to head is NULL", __func__);
    check_true_exit((list_is_singular(head)), EXIT_Q_DELETE_DUP,
                    "%s directly return due to QUEUE is singular", __func__);
    p_cur = head->next;
    p_next = p_cur->next;
    while ((p_cur != head) && (p_cur->next != head)) {
        while ((p_next != head) &&
               (strcmp(list_entry(p_cur, element_t, list)->value,
                       list_entry(p_next, element_t, list)->value) == 0)) {
            p_nn = p_next->next;
            list_del(p_next);
            q_release_element(list_entry(p_next, element_t, list));
            p_next = p_nn;
            deleted = true;
        }
        if (deleted) {
            list_del(p_cur);
            q_release_element(list_entry(p_cur, element_t, list));
            deleted = false;
        }
        p_cur = p_next;
        p_next = p_cur->next;
    }
    retval = true;
EXIT_Q_DELETE_DUP:
    return retval;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    FUNC_ENTRANCE;
    struct list_head *p_cur = NULL, *p_next = NULL;
    check_null_exit(head, EXIT_Q_SWAP, "%s occurs error due to head is NULL",
                    __func__);
    check_true_exit((list_is_singular(head)), EXIT_Q_SWAP,
                    "%s directly return due to QUEUE is singular", __func__);
    p_cur = head->next;
    while (p_cur && p_cur->next && p_cur != head && p_cur->next != head) {
        p_next = p_cur->next;
        list_del(p_next);
        list_add_tail(p_next, p_cur);
        p_cur = p_cur->next;
    }
EXIT_Q_SWAP:
    FUNC_EXIT;
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    FUNC_ENTRANCE;
    struct list_head *p_prev = NULL, *p_cur = NULL, *p_next = NULL;
    check_null_exit(head, EXIT_Q_REVERSE, "%s occurs error due to head is NULL",
                    __func__);
    p_prev = head;
    p_cur = head->next;
    p_next = p_cur->next;
    while (p_cur != head) {
        p_cur->next = p_prev;
        p_cur->prev = p_next;
        p_prev = p_cur;
        p_cur = p_next;
        p_next = p_next->next;
    }
    p_cur = head->next;
    head->next = head->prev;
    head->prev = p_cur;
EXIT_Q_REVERSE:
    FUNC_EXIT;
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */

struct list_head *q_merge(struct list_head *p_l, struct list_head *p_r)
{
    struct list_head *p_new = NULL, *p_l_head = NULL, *p_r_head = NULL;
    do {
        if ((p_l) && (p_r) &&
            (strcmp(list_entry(p_l, element_t, list)->value,
                    list_entry(p_r, element_t, list)->value) <= 0)) {
            Q_MERGE_STRCMP_LIST(p_l, p_l_head, p_new);
        } else if ((p_l) && (p_r)) {
            Q_MERGE_STRCMP_LIST(p_r, p_r_head, p_new);
        } else if ((!p_l) && (p_r)) {
            Q_MERGE_LIST(p_r, p_r_head, p_new);
        } else {
            Q_MERGE_LIST(p_l, p_l_head, p_new);
        }
    } while ((p_l) || (p_r));
    return p_new;
}

struct list_head *q_merge_sort(struct list_head *head)
{
    struct list_head *p_node = NULL;
    if ((head->next == head) && (head->prev == head)) {
        p_node = head;
        goto EXIT_Q_MERGE_SORT;
    }
    struct list_head *p_slow = head, *p_fast = head->next, *p_left = NULL,
                     *p_right = NULL;
    while ((p_fast) && (p_fast->next) && (p_fast != head) &&
           (p_fast->next != head)) {
        p_slow = p_slow->next;
        p_fast = p_fast->next->next;
    }

    if ((p_fast) && (p_fast == head)) {
        p_slow->next->prev = p_fast->prev;
        p_fast->prev->next = p_slow->next;
    } else if (p_fast) {
        p_slow->next->prev = p_fast;
        p_fast->next = p_slow->next;
    }
    p_fast = p_slow->next;
    head->prev = p_slow;
    p_slow->next = head;
    p_left = q_merge_sort(head);
    p_right = q_merge_sort(p_fast);
    p_node = q_merge(p_left, p_right);
EXIT_Q_MERGE_SORT:
    return p_node;
}

void q_sort(struct list_head *head)
{
    check_null_exit(head, EXIT_Q_SORT, "%s occurs error due to head is NULL",
                    __func__);
    check_true_exit((list_is_singular(head)), EXIT_Q_SORT,
                    "%s occurs error due to QUEUE is singular", __func__);
    head->next->prev = head->prev;
    head->prev->next = head->next;
    head->next = q_merge_sort(head->next);
    head->prev = head->next->prev;
    head->prev->next = head;
    head->next->prev = head;
EXIT_Q_SORT:
    FUNC_EXIT;
}
