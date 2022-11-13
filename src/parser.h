/*
 * Created on Sat Nov 05 2022
 *
 * Copyright (c) 2022 Adam Warren
 */
#ifndef PARSER_H
#define PARSER_H

#include "fwd.h"

#define NODE_TYPE_LIST(__item, _u) \
    __item(INVALID, _u) \
    __item(PROGRAM, _u) \
    __item(STATEMENT, _u) \
    __item(FUNCTION, _u) \
    __item(EXPRESSION, _u) \
    __item(TERM, _u) \
    __item(FACTOR, _u) 

enum node_type_e {
    NODE_TYPE_LIST(ENUM_LIST_ITEM, NODE_)
    NODE_TYPE_COUNT
};
typedef enum node_type_e node_type;

extern const char* node_type_names[NODE_TYPE_COUNT];

struct AST_node_s {
    node_type type;

    // TODO: virtual parse, debug_print, and free functions

    struct AST_node_s* next;
    struct AST_node_s* prev;
};
typedef struct AST_node_s node_t;

typedef struct AST_root_s {
    node_t node;

    ident_array_t* global_ids;
    node_t* functions;
} node_root_t;

node_root_t* parse(token_list_t* list);
void node_root_free(node_root_t* root);

void node_root_print(node_root_t* root);

#endif
