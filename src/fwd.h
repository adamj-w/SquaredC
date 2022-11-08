/*
 * Created on Sat Nov 05 2022
 *
 * Copyright (c) 2022 Adam Warren
 */
#ifndef FWD_H
#define FWD_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define ENUM_LIST_ITEM(__value, _u) _u##__value,
#define STIRNG_LIST_ITEM(__value, _) #__value,

typedef struct token_s token_t;

typedef struct AST_node_s node_t;
typedef struct AST_root_s node_root_t;

#define ZMALLOC(_type, _name) \
    _name = (_type*)malloc(sizeof(_type)); \
    memset(_name, 0, sizeof(_type))


#define BUILTIN_TYPE_LIST(__item, _uargs) \
    __item(INVALID, _uargs) \
    __item(INT, _uargs)

enum builtin_type_e {
    BUILTIN_TYPE_LIST(ENUM_LIST_ITEM, BUILTIN_)
    BUILTIN_TYPE_COUNT
};
typedef enum builtin_type_e builtin_type;
extern const char* builtin_type_names[BUILTIN_TYPE_COUNT];

#define OPERATOR_TYPE_LIST(__item, _uargs) \
    __item(INVALID, _uargs) \
    __item(ADD, _uargs) \
    __item(MINUS, _uargs) \
    __item(MULT, _uargs) \
    __item(DIVID, _uargs) \
    __item(BITWISE_COMPLEMENT, _uargs) \
    __item(LOGICAL_NOT, _uargs) 

typedef enum operator_type_e {
    OPERATOR_TYPE_LIST(ENUM_LIST_ITEM, OPERATOR_)
    OPERATOR_TYPE_COUNT
} operator_type;
extern const char* operator_type_names[OPERATOR_TYPE_COUNT];

#define KEYWORD_TYPE_LIST(__item, _uargs) \
    __item(INVALID, _uargs) \
    __item(RETURN, _uargs)

typedef enum keyword_type_e {
    KEYWORD_TYPE_LIST(ENUM_LIST_ITEM, KEYWORD_)
    KEYWORD_TYPE_COUNT
} keyword_type;
extern const char* keyword_type_names[KEYWORD_TYPE_COUNT];

#endif
