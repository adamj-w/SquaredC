/*
 * Created on Sat Nov 05 2022
 *
 * Copyright (c) 2022 Adam Warren
 */
#ifndef NODES_H
#define NODES_H

#include "parser.h"
#include "fwd.h"

typedef enum factor_type_e {
    FACTOR_INVALID,
    FACTOR_PAREN,
    FACTOR_UNARY_OP,
    FACTOR_CONST,
    FACTOR_TYPE_COUNT
} factor_type;

typedef struct AST_factor_s {
    node_t node;

    factor_type type;
    union {
        struct {
            operator_type operator;
            struct AST_factor_s* factor;
        };
        struct AST_expression_s* exp;
        unsigned int literal;
    };
} node_factor_t;

node_factor_t* parse_factor(token_t** list);
void free_factor(node_factor_t* factor);
void debug_print_node_factor(node_factor_t* factor);

typedef struct AST_term_subterm_s {
    operator_type operator;
    struct AST_factor_s* factor;

    struct AST_term_subterm_s* next;
} node_term_subterm_t;

typedef struct AST_term_s {
    node_t node;

    struct AST_factor_s* factor;

    size_t subterm_count;
    node_term_subterm_t* subterms;
} node_term_t;

node_term_t* parse_term(token_t** list);
void free_term(node_term_t* term);
void debug_print_node_term(node_term_t* term);

typedef struct AST_expression_subexpression_s {
    operator_type operator;
    struct AST_term_s* term;

    struct AST_expression_subexpression_s* next;
} node_exp_subexp_t;

typedef struct AST_expression_s {
    node_t node;

    struct AST_term_s* term;

    size_t subexp_count;
    node_exp_subexp_t* subexps;
} node_exp_t;

node_exp_t* parse_expression(token_t** list);
void free_expression(node_exp_t* exp);
void debug_print_node_expression(node_exp_t* exp);

typedef struct AST_statement_s {
    node_t node;
    
    //token_t* return_keyword;
    node_exp_t* exp;
    //token_t* semicolon;
} node_stat_t;

node_stat_t* parse_statement(token_t** list);
void debug_print_node_statement(node_stat_t* node);

typedef struct AST_function_s {
    node_t node;

    //token_t* return_type;
    builtin_type return_type;
    //token_t* function_name;
    char* function_name;

    // Do we really need to store these or just check for them
    //token_t* open_paren;
    // TODO: parameters
    //token_t* close_paren;

    //token_t* open_brace;
    node_stat_t* stat;
    //token_t* close_brace;
    
} node_func_t;

node_func_t* parse_function(token_t** list);
void debug_print_node_function(node_func_t* node);

#endif
