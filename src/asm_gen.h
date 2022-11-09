/*
 * Created on Sun Nov 06 2022
 *
 * Copyright (c) 2022 Adam Warren
 */
#ifndef ASM_GEN_H
#define ASM_GEN_H

#include <stdio.h>
#include "nodes.h"

bool generate_asm(FILE* fp, node_root_t* root);

bool ga_function(FILE* fp, node_func_t* func);
bool ga_statement(FILE* fp, node_stat_t* stat);

bool ga_subexpression(FILE* fp, node_exp_subexp_t* sub);
bool ga_expression(FILE* fp, node_exp_t* exp);

bool ga_subterm(FILE* fp, node_term_subterm_t* term);
bool ga_term(FILE* fp, node_term_t* term);

bool ga_factor(FILE* fp, node_factor_t* factor);

#endif
