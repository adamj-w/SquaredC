/*
 * Created on Sun Nov 06 2022
 *
 * Copyright (c) 2022 Adam Warren
 */
#include "asm_gen.h"

#include "parser.h"
#include "lexer.h"

#include <assert.h>

bool generate_asm(FILE* fp, node_root_t* root) {
    assert(root && root->functions && fp);

    // Need to traverse as a tree
    node_t* curr = root->functions;
    while(curr) {
        if(!ga_function(fp, (node_func_t*)curr)) return false;
        curr = curr->next;
        // switch(curr->type) {
        // case NODE_FUNCTION:
        //     node_func_t* func = (node_func_t*)curr;
        //     fprintf(fp, ".globl %s\n%s:\n", func->function_name->name, func->function_name->name);
        //     curr = (node_t*)func->stat;
        //     continue;
        // case NODE_STATEMENT:
        //     node_stat_t* stat = (node_stat_t*)curr;
        //     fprintf(fp, "\tmovl $%u, %%eax\n\tret\n", stat->exp->literal_value);
        //     break;
        // default:
        //     break;
        // }
        // curr = curr->next;
    }

    return true;
}

bool ga_function(FILE* fp, node_func_t* func) {
    fprintf(fp, ".globl %s\n%s:\n", func->function_name, func->function_name);
    return ga_statement(fp, func->stat);
}

bool ga_statement(FILE* fp, node_stat_t* stat) {
    if(!ga_expression(fp, stat->exp)) return false;
    fprintf(fp, "\tret\n");
    return true;
}

bool ga_expression(FILE* fp, node_exp_t* exp) {
    assert(fp && exp);



    printf("Failed to generate expression nodes assembly\n");
    return false;
}
