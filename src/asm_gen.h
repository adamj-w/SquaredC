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
bool ga_expression(FILE* fp, node_exp_t* exp);

#endif
