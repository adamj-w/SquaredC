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
        fact->exp = parse_exp_or(&curr);
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
            free_exp_or(factor->exp);
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
        debug_print_node_exp_or(factor->exp);
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

// ADD, expressions

node_exp_sum_t* parse_exp_sum(token_t** list) {
    token_t* curr = *list;

    node_exp_sum_t* sum;
    ZMALLOC(node_exp_sum_t, sum);
    sum->node.type = NODE_EXPRESSION;

    sum->term = parse_term(&curr);
    if(!sum->term) goto fail;
    token_t* peek = curr->next;
    if(!peek || peek->type != TOKEN_OPERATOR) goto done;
    if(peek->operator_type != OPERATOR_ADD && peek->operator_type != OPERATOR_MINUS) goto done;
    NEXT(curr);

    ZMALLOC(node_exp_sum_subexp_t, sum->subexps);
    node_exp_sum_subexp_t* subexp = sum->subexps;
    while(1) {
        assert(curr->operator_type == OPERATOR_ADD || curr->operator_type == OPERATOR_MINUS);
        subexp->operator = curr->operator_type;
        NEXT(curr);

        subexp->term = parse_term(&curr);
        if(!subexp->term) goto fail;

        peek = curr->next;
        if(!peek || peek->type != TOKEN_OPERATOR) break;
        if(peek->operator_type != OPERATOR_ADD && peek->operator_type != OPERATOR_MINUS) break;

        NEXT(curr);
        ZMALLOC(node_exp_sum_subexp_t, subexp->next);
        subexp = subexp->next;
    }    

done:
    *list = curr;
    return sum;

fail:
    free_exp_sum(sum);
    return NULL;
}

void free_exp_sum(node_exp_sum_t* sum) {
    if(sum) {
        free_term(sum->term);

        if(sum->subexps) {
            node_exp_sum_subexp_t* sub = sum->subexps;
            while(sub) {
                node_exp_sum_subexp_t* next = sub->next;
                free_term(sub->term);
                free(sub);
                sub = next;
            }
        }

        free(sum);
    }
}

void debug_print_node_exp_sum(node_exp_sum_t* sum) {
    debug_print_node_term(sum->term);

    node_exp_sum_subexp_t* sub = sum->subexps;
    while(sub) {
        printf("%s ", operator_type_names[sub->operator]);
        debug_print_node_term(sub->term);
        sub = sub->next;
    }
}

// GREATER_THAN, etc expressions

node_exp_relation_t* parse_exp_relation(token_t** list) {
    token_t* curr = *list;

    node_exp_relation_t* relation;
    ZMALLOC(node_exp_relation_t, relation);
    relation->node.type = NODE_EXPRESSION;

    relation->sum = parse_exp_sum(&curr);
    if(!relation->sum) goto fail;
    token_t* peek = curr->next;
    if(!peek || peek->type != TOKEN_OPERATOR) goto done;
    if(!IS_RELATION(peek->operator_type)) goto done;
    NEXT(curr);

    ZMALLOC(node_exp_relation_subexp_t, relation->subexps);
    node_exp_relation_subexp_t* subexp = relation->subexps;
    while(1) {
        assert(IS_RELATION(curr->operator_type));
        subexp->relation = curr->operator_type;
        NEXT(curr);

        subexp->sum = parse_exp_sum(&curr);
        if(!subexp->sum) goto fail;

        peek = curr->next;
        if(!peek || peek->type != TOKEN_OPERATOR) break;
        if(!IS_RELATION(peek->operator_type)) break;

        NEXT(curr);
        ZMALLOC(node_exp_relation_subexp_t, subexp->next);
        subexp = subexp->next;
    }    

done:
    *list = curr;
    return relation;

fail:
    free_exp_relation(relation);
    return NULL;
}

void free_exp_relation(node_exp_relation_t* relation) {
    if(relation) {
        free_exp_sum(relation->sum);

        if(relation->subexps) {
            node_exp_relation_subexp_t* sub = relation->subexps;
            while(sub) {
                node_exp_relation_subexp_t* next = sub->next;
                free_exp_sum(sub->sum);
                free(sub);
                sub = next;
            }
        }

        free(relation);
    }
}

void debug_print_node_exp_relation(node_exp_relation_t* relation) {
    debug_print_node_exp_sum(relation->sum);

    node_exp_relation_subexp_t* sub = relation->subexps;
    while(sub) {
        printf("%s ", operator_type_names[sub->relation]);
        debug_print_node_exp_sum(sub->sum);
        sub = sub->next;
    }
}

// EQUALS expressions

node_exp_equals_t* parse_exp_equals(token_t** list) {
    token_t* curr = *list;

    node_exp_equals_t* equals;
    ZMALLOC(node_exp_equals_t, equals);
    equals->node.type = NODE_EXPRESSION;

    equals->relation = parse_exp_relation(&curr);
    if(!equals->relation) goto fail;
    token_t* peek = curr->next;
    if(!peek || peek->type != TOKEN_OPERATOR) goto done;
    if(peek->operator_type != OPERATOR_EQUALS && peek->operator_type != OPERATOR_NOT_EQUAL) goto done;
    NEXT(curr);

    ZMALLOC(node_exp_equals_subexp_t, equals->subexps);
    node_exp_equals_subexp_t* subexp = equals->subexps;
    while(1) {
        assert(curr->operator_type == OPERATOR_EQUALS || curr->operator_type == OPERATOR_NOT_EQUAL);
        subexp->operator = curr->operator_type;
        NEXT(curr);

        subexp->relation = parse_exp_relation(&curr);
        if(!subexp->relation) goto fail;

        peek = curr->next;
        if(!peek || peek->type != TOKEN_OPERATOR) break;
        if(peek->operator_type != OPERATOR_EQUALS && peek->operator_type != OPERATOR_NOT_EQUAL) break;

        NEXT(curr);
        ZMALLOC(node_exp_equals_subexp_t, subexp->next);
        subexp = subexp->next;
    }    

done:
    *list = curr;
    return equals;

fail:
    free_exp_equals(equals);
    return NULL;
}

void free_exp_equals(node_exp_equals_t* equals) {
    if(equals) {
        free_exp_relation(equals->relation);

        if(equals->subexps) {
            node_exp_equals_subexp_t* sub = equals->subexps;
            while(sub) {
                node_exp_equals_subexp_t* next = sub->next;
                free_exp_relation(sub->relation);
                free(sub);
                sub = next;
            }
        }

        free(equals);
    }
}

void debug_print_node_exp_equals(node_exp_equals_t* equals) {
    debug_print_node_exp_relation(equals->relation);

    node_exp_equals_subexp_t* sub = equals->subexps;
    while(sub) {
        printf("%s ", operator_type_names[sub->operator]);
        debug_print_node_exp_relation(sub->relation);
        sub = sub->next;
    }
}

// AND expressions

node_exp_and_t* parse_exp_and(token_t** list) {
    token_t* curr = *list;

    node_exp_and_t* and;
    ZMALLOC(node_exp_and_t, and);
    and->node.type = NODE_EXPRESSION;

    and->equals = parse_exp_equals(&curr);
    if(!and->equals) goto fail;
    token_t* peek = curr->next;
    if(!peek || peek->type != TOKEN_OPERATOR) goto done;
    if(peek->operator_type != OPERATOR_AND) goto done;
    NEXT(curr);

    ZMALLOC(node_exp_and_subexp_t, and->subexps);
    node_exp_and_subexp_t* subexp = and->subexps;
    while(1) {
        assert(curr->operator_type == OPERATOR_AND);
        NEXT(curr);

        subexp->equals = parse_exp_equals(&curr);
        if(!subexp->equals) goto fail;

        peek = curr->next;
        if(!peek || peek->type != TOKEN_OPERATOR) break;
        if(peek->operator_type != OPERATOR_AND) break;

        NEXT(curr);
        ZMALLOC(node_exp_and_subexp_t, subexp->next);
        subexp = subexp->next;
    }    

done:
    *list = curr;
    return and;

fail:
    free_exp_and(and);
    return NULL;
}

void free_exp_and(node_exp_and_t* and) {
    if(and) {
        free_exp_equals(and->equals);

        if(and->subexps) {
            node_exp_and_subexp_t* sub = and->subexps;
            while(sub) {
                node_exp_and_subexp_t* next = sub->next;
                free_exp_equals(sub->equals);
                free(sub);
                sub = next;
            }
        }

        free(and);
    }
}

void debug_print_node_exp_and(node_exp_and_t* and) {
    debug_print_node_exp_equals(and->equals);

    node_exp_and_subexp_t* sub = and->subexps;
    while(sub) {
        printf("&& ");
        debug_print_node_exp_equals(sub->equals);
        sub = sub->next;
    }
}

// OR expressions

node_exp_or_t* parse_exp_or(token_t** list) {
    token_t* curr = *list;

    node_exp_or_t* exp;
    ZMALLOC(node_exp_or_t, exp);
    exp->node.type = NODE_EXPRESSION;

    exp->and_exp = parse_exp_and(&curr);
    if(!exp->and_exp) goto fail;
    if(PEEK(curr)->type != TOKEN_OPERATOR) goto done;
    NEXT(curr);

    ZMALLOC(node_exp_or_subexp_t, exp->subexps);
    node_exp_or_subexp_t* subexp = exp->subexps;
    while(1) {
        assert(curr->operator_type == OPERATOR_OR);
        NEXT(curr);

        subexp->and_exp = parse_exp_and(&curr);
        if(!subexp->and_exp) goto fail;

        if(PEEK(curr)->type != TOKEN_OPERATOR) break;
        NEXT(curr);
        ZMALLOC(node_exp_or_subexp_t, subexp->next);
        subexp = subexp->next;
    }    

done:
    *list = curr;
    return exp;

fail:
    free_exp_or(exp);
    return NULL;
}

void free_exp_or(node_exp_or_t* exp) {
    if(exp) {
        free_exp_and(exp->and_exp);

        if(exp->subexps) {
            node_exp_or_subexp_t* sub = exp->subexps;
            while(sub) {
                node_exp_or_subexp_t* next = sub->next;
                free_exp_and(sub->and_exp);
                free(sub);
                sub = next;
            }
        }

        free(exp);
    }
}

void debug_print_node_exp_or(node_exp_or_t* exp) {
    debug_print_node_exp_and(exp->and_exp);

    node_exp_or_subexp_t* sub = exp->subexps;
    while(sub) {
        printf("|| ");
        debug_print_node_exp_and(sub->and_exp);
        sub = sub->next;
    }
}

// Expressions

node_exp_t* parse_exp(token_t** list) {
    token_t* curr = *list;
    node_exp_t* exp;
    ZMALLOC(node_exp_t, exp);
    exp->node.type = NODE_EXPRESSION;

    if(curr->type == TOKEN_IDENTIFIER) {
        exp->type = EXP_ASSIGN;
        exp->id = curr->name;
        curr->name_owner = 0;
        NEXT(curr);

        if(curr->type != TOKEN_ASSIGN) goto fail;
        NEXT(curr);

        exp->exp = parse_exp(&curr);
        if(!exp->exp) goto fail;
    } else {
        exp->type = EXP_LOGICAL;
        exp->or_exp = parse_exp_or(&curr);
        if(!exp->or_exp) goto fail;
    }

    *list = curr;
    return exp;

fail:
    free_exp(exp);
    return NULL;
}

void free_exp(node_exp_t* exp) {
    if(exp) {
        if(exp->type == EXP_ASSIGN) {
            free_exp(exp->exp); // TODO: free id
        } else if(exp->type == EXP_LOGICAL) {
            free_exp_or(exp->or_exp);
        }

        free(exp);
    }
}

void debug_print_node_exp(node_exp_t* exp) {
    if(exp->type == EXP_LOGICAL) {
        debug_print_node_exp_or(exp->or_exp);
    } else if(exp->type == EXP_ASSIGN) {
        printf("%s = ", exp->id);
        debug_print_node_exp(exp->exp);
    } else {
        printf("INV ");
    }
}

// Statement

node_stat_t* parse_statement(token_t** list) {
    token_t* curr = *list;

    node_stat_t* out;
    ZMALLOC(node_stat_t, out);
    out->node.type = NODE_STATEMENT;

    if(curr->type == TOKEN_KEYWORD && curr->keyword_type == KEYWORD_RETURN) {
        out->type = STATEMENT_RETURN;
        NEXT(curr);

        out->exp = parse_exp(&curr);
        if(!out->exp) goto fail;
        NEXT(curr);

        if(curr->type != TOKEN_SEMICOLON) goto fail;
    } else if(curr->type == TOKEN_BUILTIN_TYPE) {
        out->type = STATEMENT_DECLARE;

        out->builtin_type = curr->builtin_type;
        NEXT(curr);

        if(curr->type != TOKEN_IDENTIFIER) goto fail;
        out->variable = curr->name;
        curr->name_owner = 0;
        NEXT(curr);

        if(curr->type == TOKEN_SEMICOLON) goto out;
        if(curr->type != TOKEN_ASSIGN) goto fail;
        NEXT(curr);

        out->exp = parse_exp(&curr);
        if(!out->exp) goto fail;
        NEXT(curr);

        if(curr->type != TOKEN_SEMICOLON) goto fail;

    } else {
        out->type = STATEMENT_EXP;
        out->exp = parse_exp(&curr);
        if(!out->exp) goto fail;
        NEXT(curr);

        if(curr->type != TOKEN_SEMICOLON) goto fail;        
    }

out:

    if(PEEK(curr)->type == TOKEN_CLOSE_BRACE) goto done;
    NEXT(curr);

    out->next = parse_statement(&curr);
    if(!out->next) goto fail;    
    
done:
    *list = curr;
    return out;

fail:
    free(out);
    return NULL;
}

void debug_print_node_statement(node_stat_t* node) {
    printf("\tRET "); // TODO:
    debug_print_node_exp(node->exp);
    printf("\n");
}

node_func_t* parse_function(token_t** list) {
    token_t* curr = *list;

    node_func_t* out;
    ZMALLOC(node_func_t, out);
    out->node.type = NODE_FUNCTION;

    if(curr->type != TOKEN_BUILTIN_TYPE) goto fail;
    out->return_type = curr->builtin_type;
    NEXT(curr);

    if(curr->type != TOKEN_IDENTIFIER) goto fail;
    out->function_name = curr->name;
    curr->name_owner = 0;
    NEXT(curr);

    if(curr->type != TOKEN_OPEN_PAREN) goto fail;
    NEXT(curr);

    // TODO: parameters

    if(curr->type != TOKEN_CLOSE_PAREN) goto fail;
    NEXT(curr);
    
    if(curr->type != TOKEN_OPEN_BRACE) goto fail;
    NEXT(curr);

    out->stat = parse_statement(&curr);
    if(!out->stat) goto fail;
    NEXT(curr);
    
    if(curr->type != TOKEN_CLOSE_BRACE) goto fail;

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
