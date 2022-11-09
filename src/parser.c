/*
 * Created on Sat Nov 05 2022
 *
 * Copyright (c) 2022 Adam Warren
 */
#include "parser.h"
#include "nodes.h"

#include "lexer.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

node_root_t* parse(token_t* tokens) {
    assert(tokens);

    node_root_t* root;
    ZMALLOC(node_root_t, root);

    token_t* curr = tokens;
    root->functions = (node_t*)parse_function(&curr);

    if(!root->functions) {
        free(root);
        return NULL;
    }

    return root;
}

void free_root_node(node_root_t* root) {
    assert(root);

    // TODO: names are still left floating for identifiers
    if(root->functions) {
        node_t* curr = root->functions;
        while(curr) {
            node_t* next = curr->next;
            free(curr);
            curr = next;
        }
    }

    free(root);
}

void debug_print_node_tree(node_root_t* root) {
    assert(root);

    node_t* curr = root->functions;
    while(curr) {
        switch(curr->type) {
        case NODE_FUNCTION:
            debug_print_node_function((node_func_t*)curr);
            break;
        default:
            printf("Invalid Node\n");
            break;
        }
        curr = curr->next;
    }
}

#define NEXT(_curr) if(!_curr->next) goto fail; else _curr = _curr->next
#define PEEK(_curr) _curr->next && _curr->next

node_factor_t* parse_factor(token_t** list) {
    token_t* curr = *list;
    node_factor_t* fact;
    ZMALLOC(node_factor_t, fact);
    fact->node.type = NODE_FACTOR;

    if(curr->type == TOKEN_OPEN_PAREN) {
        fact->type = FACTOR_PAREN;

        NEXT(curr);
        fact->exp = parse_expression(&curr);
        if(!fact->exp) goto fail;
        NEXT(curr);

        if(curr->type != TOKEN_CLOSE_PAREN) goto fail;
    } else if(curr->type == TOKEN_OPERATOR) {
        fact->type = FACTOR_UNARY_OP;
        if(curr->operator_type != OPERATOR_BITWISE_COMPLEMENT &&
            curr->operator_type != OPERATOR_LOGICAL_NOT &&
            curr->operator_type != OPERATOR_MINUS) goto fail;

        fact->operator = curr->operator_type;
        NEXT(curr);

        fact->factor = parse_factor(&curr);
        if(!fact->factor) goto fail;
    } else if(curr->type == TOKEN_LITERAL) {
        fact->type = FACTOR_CONST;
        fact->literal = curr->literal_value;
    } else goto fail;

    *list = curr;
    return fact;

fail:
    free_factor(fact);
    return NULL;
}

void free_factor(node_factor_t* factor) {
    if(factor) {
        if(factor->type == FACTOR_PAREN) {
            free_expression(factor->exp);
        } else if(factor->type == FACTOR_UNARY_OP) {
            free_factor(factor->factor);
        }

        free(factor);
    }
}

void debug_print_node_factor(node_factor_t* factor) {
    if(factor->type == FACTOR_CONST) {
        printf("%u ", factor->literal);
    } else if(factor->type == FACTOR_PAREN) {
        printf("(");
        debug_print_node_expression(factor->exp);
        printf(")");
    } else if(factor->type == FACTOR_UNARY_OP) {
        printf("%s ", operator_type_names[factor->operator]);
        debug_print_node_factor(factor->factor);
    } else {
        printf("ERR");
    }
}

node_term_t* parse_term(token_t** list) {
    token_t* curr = *list;

    node_term_t* term;
    ZMALLOC(node_term_t, term);
    term->node.type = NODE_TERM;

    term->factor = parse_factor(&curr);
    if(!term->factor) goto fail;
    token_t* peek = curr->next;
    if(!peek || peek->type != TOKEN_OPERATOR) goto done;
    if(peek->operator_type != OPERATOR_MULT && peek->operator_type != OPERATOR_DIVID) goto done;
    NEXT(curr);

    ZMALLOC(node_term_subterm_t, term->subterms);
    node_term_subterm_t* sub = term->subterms;
    while(1) {
        assert(curr->operator_type == OPERATOR_MULT || curr->operator_type == OPERATOR_DIVID);
        sub->operator = curr->operator_type;
        NEXT(curr);

        sub->factor = parse_factor(&curr);
        if(!sub->factor) goto fail;
        peek = curr->next;

        if(!peek || peek->type != TOKEN_OPERATOR) break;
        if(peek->operator_type != OPERATOR_MULT && peek->operator_type != OPERATOR_DIVID) break;

        NEXT(curr);
        ZMALLOC(node_term_subterm_t, sub->next);
        sub = sub->next;
    }

done:
    *list = curr;
    return term;

fail:
    free_term(term);
    return NULL;
}

void free_term(node_term_t* term) {
    if(term) {
        free_factor(term->factor);

        if(term->subterms) {
            node_term_subterm_t* sub = term->subterms;
            while(sub) {
                node_term_subterm_t* next= sub->next;
                free_factor(sub->factor);
                free(sub);
                sub = next;
            }
        }

        free(term);
    }
}

void debug_print_node_term(node_term_t* term) {
    debug_print_node_factor(term->factor);

    node_term_subterm_t* sub = term->subterms;
    while(sub) {
        printf("%s ", operator_type_names[sub->operator]);
        debug_print_node_factor(sub->factor);
        sub = sub->next;
    }
}

node_exp_t* parse_expression(token_t** list) {
    token_t* curr = *list;

    node_exp_t* exp;
    ZMALLOC(node_exp_t, exp);
    exp->node.type = NODE_EXPRESSION;

    exp->term = parse_term(&curr);
    if(!exp->term) goto fail;
    if(PEEK(curr)->type != TOKEN_OPERATOR) goto ret;
    NEXT(curr);

    ZMALLOC(node_exp_subexp_t, exp->subexps);
    node_exp_subexp_t* subexp = exp->subexps;
    while(1) {
        assert(curr->operator_type == OPERATOR_ADD || curr->operator_type == OPERATOR_MINUS);
        subexp->operator = curr->operator_type;
        NEXT(curr);

        subexp->term = parse_term(&curr);
        if(!subexp->term) goto fail;

        if(PEEK(curr)->type != TOKEN_OPERATOR) break;
        NEXT(curr);
        ZMALLOC(node_exp_subexp_t, subexp->next);
        subexp = subexp->next;
    }    

ret:
    *list = curr;
    return exp;

fail:
    free_expression(exp);
    return NULL;
}

void free_expression(node_exp_t* exp) {
    if(exp) {
        free_term(exp->term);

        if(exp->subexps) {
            node_exp_subexp_t* sub = exp->subexps;
            while(sub) {
                node_exp_subexp_t* next = sub->next;
                free_term(sub->term);
                free(sub);
                sub = next;
            }
        }

        free(exp);
    }
}

void debug_print_node_expression(node_exp_t* exp) {
    debug_print_node_term(exp->term);

    node_exp_subexp_t* sub = exp->subexps;
    while(sub) {
        printf("%s ", operator_type_names[sub->operator]);
        debug_print_node_term(sub->term);
        sub = sub->next;
    }
}

node_stat_t* parse_statement(token_t** list) {
    token_t* curr = *list;

    node_stat_t* out;
    ZMALLOC(node_stat_t, out);
    out->node.type = NODE_STATEMENT;

    if(curr->type != TOKEN_KEYWORD && curr->keyword_type != KEYWORD_RETURN) goto fail;
    // out->return_keyword = curr;
    NEXT(curr);

    out->exp = parse_expression(&curr);
    if(!out->exp) goto fail;
    NEXT(curr);

    if(curr->type != TOKEN_SEMICOLON) goto fail;
    // out->semicolon = curr;
    
    *list = curr;
    return out;

fail:
    free(out);
    return NULL;
}

void debug_print_node_statement(node_stat_t* node) {
    printf("\tRET ");
    debug_print_node_expression(node->exp);
    printf("\n");
}

node_func_t* parse_function(token_t** list) {
    token_t* curr = *list;

    node_func_t* out;
    ZMALLOC(node_func_t, out);
    out->node.type = NODE_FUNCTION;

    // TODO: do range checking for keyword type
    if(curr->type != TOKEN_BUILTIN_TYPE) goto fail;
    out->return_type = curr->builtin_type;
    NEXT(curr);

    if(curr->type != TOKEN_IDENTIFIER) goto fail;
    out->function_name = curr->name;
    curr->name_owner = 0;
    NEXT(curr);

    if(curr->type != TOKEN_OPEN_PAREN) goto fail;
    // out->open_paren = curr;
    NEXT(curr);

    if(curr->type != TOKEN_CLOSE_PAREN) goto fail;
    // out->close_paren = curr;
    NEXT(curr);
    
    if(curr->type != TOKEN_OPEN_BRACE) goto fail;
    // out->open_brace = curr;
    NEXT(curr);

    out->stat = parse_statement(&curr);
    if(!out->stat) goto fail;
    NEXT(curr);
    
    if(curr->type != TOKEN_CLOSE_BRACE) goto fail;
    // out->close_brace = curr;

    *list = curr;
    return out;
fail:
    free(out);
    return NULL;
}

void debug_print_node_function(node_func_t* node) {
    printf("Name: \"%s\", Returns: %s, Params: \"\", Body: \n", 
        node->function_name, 
        builtin_type_names[node->return_type]);
    
    debug_print_node_statement(node->stat);
}
