/*
 * Created on Sat Nov 05 2022
 *
 * Copyright (c) 2022 Adam Warren
 */
#ifndef NODES_H
#define NODES_H

#include "parser.h"
#include "fwd.h"

typedef struct parse_params {
    ident_array_t* token_ids;
    ident_array_t* global_ids;
    ident_array_t* function_ids; 
} parse_args_t;

// UNARY OPS, LITERALS, and PARENS, VARS

typedef enum factor_type_e {
    FACTOR_INVALID,
    FACTOR_PAREN,
    FACTOR_UNARY_OP,
    FACTOR_CONST,
    FACTOR_VAR,
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
        struct AST_exp_or_s* exp;
        unsigned int literal;
        ident_t* id;
    };
} node_factor_t;

node_factor_t* parse_factor(parse_args_t* args, token_t** list);
void free_factor(node_factor_t* factor);
void debug_print_node_factor(node_factor_t* factor);

// MULT and DIFF

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

node_term_t* parse_term(parse_args_t* args, token_t** list);
void free_term(node_term_t* term);
void debug_print_node_term(node_term_t* term);

// SUM and DIFF

typedef struct AST_exp_sum_subexpression_s {
    operator_type operator;
    struct AST_term_s* term;
    struct AST_exp_sum_subexpression_s* next;
} node_exp_sum_subexp_t;

typedef struct AST_exp_sum_s {
    node_t node;

    struct AST_term_s* term;

    node_exp_sum_subexp_t* subexps;
} node_exp_sum_t;

node_exp_sum_t* parse_exp_sum(parse_args_t* args, token_t** list);
void free_exp_sum(node_exp_sum_t* exp);
void debug_print_node_exp_sum(node_exp_sum_t* exp);

// GREATER_THAN, LESS_THAN, etc

typedef struct AST_exp_relation_subexpression_s {
    operator_type relation;
    struct AST_exp_sum_s* sum;
    struct AST_exp_relation_subexpression_s* next;
} node_exp_relation_subexp_t;

typedef struct AST_exp_relation_s {
    node_t node;

    struct AST_exp_sum_s* sum;

    node_exp_relation_subexp_t* subexps;
} node_exp_relation_t;

node_exp_relation_t* parse_exp_relation(parse_args_t* args, token_t** list);
void free_exp_relation(node_exp_relation_t* exp);
void debug_print_node_exp_relation(node_exp_relation_t* exp);

// EQUAL and NOT_EQUAL

typedef struct AST_exp_equals_subexpression_s {
    operator_type operator;
    struct AST_exp_relation_s* relation;
    struct AST_exp_equals_subexpression_s* next;
} node_exp_equals_subexp_t;

typedef struct AST_exp_equals_s {
    node_t node;

    struct AST_exp_relation_s* relation;

    node_exp_equals_subexp_t* subexps;
} node_exp_equals_t;

node_exp_equals_t* parse_exp_equals(parse_args_t* args, token_t** list);
void free_exp_equals(node_exp_equals_t* exp);
void debug_print_node_exp_equals(node_exp_equals_t* exp);

// AND

typedef struct AST_exp_and_subexpression_s {
    struct AST_exp_equals_s* equals;
    struct AST_exp_and_subexpression_s* next;
} node_exp_and_subexp_t;

typedef struct AST_exp_and_s {
    node_t node;

    struct AST_exp_equals_s* equals;

    node_exp_and_subexp_t* subexps;
} node_exp_and_t;

node_exp_and_t* parse_exp_and(parse_args_t* args, token_t** list);
void free_exp_and(node_exp_and_t* exp);
void debug_print_node_exp_and(node_exp_and_t* exp);

// OR

typedef struct AST_exp_or_subexpression_s {
    struct AST_exp_and_s* and_exp;
    struct AST_exp_or_subexpression_s* next;
} node_exp_or_subexp_t;

typedef struct AST_exp_or_s {
    node_t node;

    struct AST_exp_and_s* and_exp;

    node_exp_or_subexp_t* subexps;
} node_exp_or_t;

node_exp_or_t* parse_exp_or(parse_args_t* args, token_t** list);
void free_exp_or(node_exp_or_t* exp);
void debug_print_node_exp_or(node_exp_or_t* exp);

// ASSIGN or OTHER_EXP

typedef enum expression_type_s {
    EXP_INVALID,
    EXP_ASSIGN,
    EXP_LOGICAL,
    EXP_TYPE_COUNT,
} node_exp_type;

typedef struct AST_expression_s {
    node_t node;
    node_exp_type type;

    union {
        struct {
            ident_t* id; // TODO: identifier stuff
            struct AST_expression_s* exp;
        };
        node_exp_or_t* or_exp;
    };
} node_exp_t;

node_exp_t* parse_exp(parse_args_t* args, token_t** list);
void free_exp(node_exp_t* exp);
void debug_print_node_exp(node_exp_t* exp);

// Statement (actual code)

typedef enum statement_type_e {
    STATEMENT_INVALID,
    STATEMENT_RETURN,
    STATEMENT_DECLARE,
    STATEMENT_EXP,
    STATEMENT_TYPE_COUNT,
} stat_type;

typedef struct AST_statement_s {
    node_t node;
    stat_type type;

    builtin_type builtin_type;
    ident_t* variable;
    node_exp_t* exp;

    struct AST_statement_s* next;
} node_stat_t;

node_stat_t* parse_statement(parse_args_t* args, token_t** list);
void debug_print_node_statement(node_stat_t* node);

typedef struct AST_function_s {
    node_t node;

    builtin_type return_type;
    ident_t* function_id;

    // TODO: parameters

    ident_array_t* ids;
    node_stat_t* stat;
    
} node_func_t;

node_func_t* parse_function(parse_args_t* args, token_t** list);
void debug_print_node_function(node_func_t* node);

#endif
