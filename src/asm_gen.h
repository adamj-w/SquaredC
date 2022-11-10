/*
 * Created on Sun Nov 06 2022
 *
 * Copyright (c) 2022 Adam Warren
 */
#ifndef ASM_GEN_H
#define ASM_GEN_H

#include <stdio.h>
#include "nodes.h"

typedef struct ga_data_s {
    FILE* fp;
    size_t label_index;
} ga_data_t;

bool generate_asm(FILE* fp, node_root_t* root);

bool ga_function(ga_data_t* data, node_func_t* func);
bool ga_statement(ga_data_t* data, node_stat_t* stat);

//bool ga_subexpression(ga_data_t* data, node_exp_subexp_t* sub);
bool ga_expression(ga_data_t* data, node_exp_t* exp);

bool ga_exp_and(ga_data_t* data, node_exp_and_t* and);

bool ga_exp_equals(ga_data_t* data, node_exp_equals_t* equals);

bool ga_exp_relation(ga_data_t* data, node_exp_relation_t* relation);

bool ga_subexp_sum(ga_data_t* data, node_exp_sum_subexp_t* sub);
bool ga_exp_sum(ga_data_t* data, node_exp_sum_t* sum);

bool ga_subterm(ga_data_t* data, node_term_subterm_t* term);
bool ga_term(ga_data_t* data, node_term_t* term);

bool ga_factor(ga_data_t* data, node_factor_t* factor);

#endif
