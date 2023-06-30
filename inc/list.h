#pragma once

#include <stddef.h>

typedef struct list_node_s {
    struct list_node_s* next;
    void* value;
} list_node_t;

typedef struct list_s {
    list_node_t* head;
    size_t len;
} list_t;

void list_init(list_t* list);

void list_steal(list_t* dst, list_t* src);

int list_is_empty(list_t* list);

int list_contains(list_t* list, void* value);

int list_add(list_t* list, void* value);

void* list_pop(list_t* list);

void list_remove(list_t* list, void* value);
