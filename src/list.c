#include <stdlib.h>
#include <string.h>

#include "list.h"

void list_init(list_t* list)
{
    list->head = NULL;
    list->len = 0;
}

void list_steal(list_t* dst, list_t* src)
{
    memcpy(dst, src, sizeof(*src));
    list_init(src);
}

int list_is_empty(list_t* list)
{
    return list->len == 0;
}

int list_contains(list_t* list, void* value)
{
    list_node_t* iter = list->head;

    while (iter != NULL) {
        if (iter->value == value) {
            return 1;
        }
        iter = iter->next;
    }

    return 0;
}

int list_add(list_t* list, void* value)
{
    list_node_t* node = NULL;

    node = malloc(sizeof(*node));
    if (node == NULL) {
        return -1;
    }

    node->next = list->head;
    node->value = value;
    list->head = node;
    list->len += 1;

    return 0;
}

void* list_pop(list_t* list)
{
    if (list->len == 0) {
        return NULL;
    }

    list_node_t* prev_head = list->head;
    void* value = prev_head->value;

    list->head = prev_head->next;
    list->len -= 1;
    free(prev_head);

    return value;
}

void list_remove(list_t* list, void* value)
{
    list_node_t* iter = list->head;

    if (iter->value == value) {
        list->head = iter->next;
        list->len -= 1;
        free(iter);
        return;
    }

    while (iter->next != NULL) {
        if (iter->next->value == value) {
            list_node_t* removed = iter->next;
            iter->next = iter->next->next;
            free(removed);
            list->len -= 1;
            return;
        }
        iter = iter->next;
    }
}
