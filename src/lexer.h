/*
 * Created on Sat Nov 05 2022
 *
 * Copyright (c) 2022 Adam Warren
 */
#ifndef LEXER_H
#define LEXER_H

#include "fwd.h"

#define TOKEN_TYPE_LIST(__item, _uargs) \
    __item(INVALID_TOKEN, _uargs) \
    __item(OPEN_BRACE, _uargs) \
    __item(CLOSE_BRACE, _uargs) \
    __item(OPEN_PAREN, _uargs) \
    __item(CLOSE_PAREN, _uargs) \
    __item(SEMICOLON, _uargs) \
    __item(ASSIGN, _uargs) \
    __item(BUILTIN_TYPE, _uargs) \
    __item(OPERATOR, _uargs) \
    __item(KEYWORD, _uargs) \
    __item(IDENTIFIER, _uargs) \
    __item(LITERAL, _uargs)

#define TOKEN_TYPE_ENUM(entry) TOKEN_#entry,
enum token_type_e {
    TOKEN_TYPE_LIST(ENUM_LIST_ITEM, TOKEN_)
    TOKEN_TYPE_COUNT
};
typedef enum token_type_e token_type;
extern const char* token_type_names[TOKEN_TYPE_COUNT];

struct token_s {
    token_type type;
    union {
        builtin_type builtin_type;
        keyword_type keyword_type;
        operator_type operator_type;
        size_t id_index;
        unsigned int literal_value;
    };
    struct token_s* next;
    struct token_s* prev;
};
typedef struct token_s token_t;

typedef struct token_list_s {
    token_t* head;
    ident_array_t* ids;
} token_list_t;

token_list_t* lex(const char* content, size_t len);

void free_token_list(token_list_t* list);

void debug_print_list(token_list_t* list);

#endif
