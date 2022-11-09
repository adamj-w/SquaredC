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

    return true; // TODO: change back
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

// sum eax, with next term store in eax
bool ga_subexpression(FILE* fp, node_exp_subexp_t* sub) {
    fputs("\tmovl\t%eax, %ecx\n", fp);
    if(!ga_term(fp, sub->term)) return false;

    if(sub->operator == OPERATOR_ADD) {
        fputs("\taddl\t%ecx, %eax\n", fp);
        return true;
    } else if(sub->operator == OPERATOR_MINUS) {
        fputs("\tsubl\t%eax, %ecx\n", fp);
        fputs("\txchg\t%eax, %ecx\n", fp);
        return true;
    }

    return false;
}

bool ga_expression(FILE* fp, node_exp_t* exp) {
    if(!ga_term(fp, exp->term)) return false;
    //fputs("\tpush\t%eax\n", fp);

    node_exp_subexp_t* sub = exp->subexps;
    while(sub) {
        if(!ga_subexpression(fp, sub)) return false;
        sub = sub->next;
    }

    return true;
}

// mult eax, with next subterm (can't use ecx)
bool ga_subterm(FILE* fp, node_term_subterm_t* term) {
    fputs("\tmovl\t%eax, %ebx\n", fp);
    ga_factor(fp, term->factor);

    if(term->operator == OPERATOR_MULT) {
        fputs("\timul\t%ebx, %eax\n", fp);
        return true;
    } else if(term->operator == OPERATOR_DIVID) {
        fputs("\txchg\t%eax, %ebx\n\tcdq\n", fp);
        fputs("\tidiv\t%ebx\n", fp);
        return true;
    }

    return false;
}

// save result in eax
bool ga_term(FILE* fp, node_term_t* term) {
    if(!ga_factor(fp, term->factor)) return false;
    
    node_term_subterm_t* sub = term->subterms;
    while(sub) {
        if(!ga_subterm(fp, sub)) return false;
        sub = sub->next;
    }

    return true;
}

// set eax to factors value, save ebx, ecx
bool ga_factor(FILE* fp, node_factor_t* factor) {
    if(factor->type == FACTOR_CONST) {
        fprintf(fp, "\tmovl\t$%u, %%eax\n", factor->literal);
        return true;
    } else if(factor->type == FACTOR_UNARY_OP) {
        if(!ga_factor(fp, factor->factor)) return false;
        switch(factor->operator) {
        case OPERATOR_BITWISE_COMPLEMENT:
            fputs("\tnot\t\t%eax\n", fp);
            return true;
        case OPERATOR_MINUS:
            fputs("\tneg\t\t%eax\n", fp);
            return true;
        case OPERATOR_LOGICAL_NOT:
            fputs("\tcmpl\t$0, %eax\n\tmovl\t$0, %eax\n\tsete\t%al\n", fp);
            return true;
        default:
            return false;
        }
    } else if(factor->type == FACTOR_PAREN) {
        fputs("\tpush\t%ebx\n\tpush\t%ecx\n", fp);
        if(!ga_expression(fp, factor->exp)) return false;
        fputs("\tpop\t\t%ecx\n\tpop\t\t%ebx\n", fp);
        return true;
    }

    return false;
}
