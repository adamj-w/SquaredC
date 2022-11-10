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

    ga_data_t data;
    data.fp = fp;
    data.label_index = 0;

    // Need to traverse as a tree
    node_t* curr = root->functions;
    while(curr) {
        if(!ga_function(&data, (node_func_t*)curr)) return false;
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

bool ga_function(ga_data_t* data, node_func_t* func) {
    fprintf(data->fp, ".globl %s\n%s:\n", func->function_name, func->function_name);
    return ga_statement(data, func->stat);
}

bool ga_statement(ga_data_t* data, node_stat_t* stat) {
    if(!ga_expression(data, stat->exp)) return false;
    fprintf(data->fp, "\tret\n");
    return true;
}

// bool ga_subexpression(ga_data_t* data, node_exp_subexp_t* sub) {

// }

bool ga_expression(ga_data_t* data, node_exp_t* exp) {
    if(!ga_exp_and(data, exp->and_exp)) return false;

    // TODO:
    // node_exp_subexp_t* sub = exp->subexps;
    // while(sub) {

    //     fputs("\tpush\t%eax\n");
    //     if(!ga_exp_and(data, exp->and_exp)) return false;
    //     fputs("\tpop\t\t%ebx\n");
    // }
    
    return true;
}

bool ga_exp_and(ga_data_t* data, node_exp_and_t* and) {
    if(!ga_exp_equals(data, and->equals)) return false;

    // TODO:

    return true;
}

bool ga_exp_equals(ga_data_t* data, node_exp_equals_t* equals) {
    if(!ga_exp_relation(data, equals->relation)) return false;

    node_exp_equals_subexp_t* sub = equals->subexps;
    while(sub) {
        fputs("\tpush\t%eax\n", data->fp);
        if(!ga_exp_relation(data, sub->relation)) return false;

        if(sub->operator == OPERATOR_EQUALS) {
            fputs("\tpop\t\t%ecx\n\tcmpl\t%eax, %ecx\n\tmovl\t$0, %eax\n\tsete\t%al\n", data->fp);
        } else
            return false;

        sub = sub->next;
    }

    return true;
}

bool ga_exp_relation(ga_data_t* data, node_exp_relation_t* relation) {
    if(!ga_exp_sum(data, relation->sum)) return false;

    node_exp_relation_subexp_t* sub = relation->subexps;
    while(sub) {
        // TODO:

        sub = sub->next;
    }

    return true;
}

// sum eax, with next term store in eax
bool ga_subexp_sum(ga_data_t* data, node_exp_sum_subexp_t* sub) {
    fputs("\tmovl\t%eax, %ecx\n", data->fp);
    if(!ga_term(data, sub->term)) return false;

    if(sub->operator == OPERATOR_ADD) {
        fputs("\taddl\t%ecx, %eax\n", data->fp);
        return true;
    } else if(sub->operator == OPERATOR_MINUS) {
        fputs("\tsubl\t%eax, %ecx\n", data->fp);
        fputs("\txchg\t%eax, %ecx\n", data->fp);
        return true;
    }

    return false;
}

bool ga_exp_sum(ga_data_t* data, node_exp_sum_t* sum) {
    if(!ga_term(data, sum->term)) return false;
    //fputs("\tpush\t%eax\n", fp);

    node_exp_sum_subexp_t* sub = sum->subexps;
    while(sub) {
        fputs("\tpush\t%eax\n", data->fp);
        if(!ga_term(data, sub->term)) return false;
        fputs("\tpop\t\t%ecx\n", data->fp);

        if(sub->operator == OPERATOR_ADD) {
            fputs("\taddl\t%ecx, %eax\n", data->fp);
        } else if(sub->operator == OPERATOR_MINUS) {
            fputs("\tsubl\t%eax, %ecx\n", data->fp);
            fputs("\txchg\t%eax, %ecx\n", data->fp);
        }

        sub = sub->next;
    }

    return true;
}

// mult eax, with next subterm (can't use ecx)
bool ga_subterm(ga_data_t* data, node_term_subterm_t* term) {
    fputs("\tmovl\t%eax, %ebx\n", data->fp);
    ga_factor(data, term->factor);

    if(term->operator == OPERATOR_MULT) {
        fputs("\timul\t%ebx, %eax\n", data->fp);
        return true;
    } else if(term->operator == OPERATOR_DIVID) {
        fputs("\txchg\t%eax, %ebx\n\tcdq\n", data->fp);
        fputs("\tidiv\t%ebx\n", data->fp);
        return true;
    }

    return false;
}

// save result in eax
bool ga_term(ga_data_t* data, node_term_t* term) {
    if(!ga_factor(data, term->factor)) return false;
    
    node_term_subterm_t* sub = term->subterms;
    while(sub) {
        if(!ga_subterm(data, sub)) return false;
        sub = sub->next;
    }

    return true;
}

// set eax to factors value, save ebx, ecx
bool ga_factor(ga_data_t* data, node_factor_t* factor) {
    if(factor->type == FACTOR_CONST) {
        fprintf(data->fp, "\tmovl\t$%u, %%eax\n", factor->literal);
        return true;
    } else if(factor->type == FACTOR_UNARY_OP) {
        if(!ga_factor(data, factor->factor)) return false;
        switch(factor->operator) {
        case OPERATOR_BITWISE_COMPLEMENT:
            fputs("\tnot\t\t%eax\n", data->fp);
            return true;
        case OPERATOR_MINUS:
            fputs("\tneg\t\t%eax\n", data->fp);
            return true;
        case OPERATOR_LOGICAL_NOT:
            fputs("\tcmpl\t$0, %eax\n\tmovl\t$0, %eax\n\tsete\t%al\n", data->fp);
            return true;
        default:
            return false;
        }
    } else if(factor->type == FACTOR_PAREN) {
        fputs("\tpush\t%ebx\n\tpush\t%ecx\n", data->fp);
        if(!ga_expression(data, factor->exp)) return false;
        fputs("\tpop\t\t%ecx\n\tpop\t\t%ebx\n", data->fp);
        return true;
    }

    return false;
}
