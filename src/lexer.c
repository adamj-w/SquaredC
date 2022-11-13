/*
 * Created on Sat Nov 05 2022
 *
 * Copyright (c) 2022 Adam Warren
 */
#include "lexer.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "fwd.h"

#include "identifier.h"

const char* token_type_names[TOKEN_TYPE_COUNT] = {
    TOKEN_TYPE_LIST(STIRNG_LIST_ITEM, _)
};

const char* builtin_type_names[BUILTIN_TYPE_COUNT] = {
    BUILTIN_TYPE_LIST(STIRNG_LIST_ITEM, _)
};

const char* operator_type_names[OPERATOR_TYPE_COUNT] = {
    OPERATOR_TYPE_LIST(STIRNG_LIST_ITEM, _)
};

const char* keyword_type_names[KEYWORD_TYPE_COUNT] = {
    KEYWORD_TYPE_LIST(STIRNG_LIST_ITEM, _)
};

#define CREATE_NEXT(_curr) \
            ZMALLOC(token_t, _curr->next); \
            _curr->next->prev = _curr; \
            _curr = _curr->next

token_list_t* lex(const char* content, size_t len) {
    assert(len > 0);
    assert(content);

    token_list_t* list = malloc(sizeof(token_list_t));
    assert(list);

    list->ids = create_id_arr_c(5);

    ZMALLOC(token_t, list->head);
    token_t* curr = list->head;
    for(size_t i = 0; i < len; ++i) {
        char c = content[i], n = 0;
        if(i < len) n = content[i + 1];
        if(c == '\0') break;

        if(c == ' ' || c == '\n') continue; // end current token

        // look for structural elements

        if(c == '{') {
            curr->type = TOKEN_OPEN_BRACE;
            CREATE_NEXT(curr);
            continue;
        }
        
        if(c == '}') {
            curr->type = TOKEN_CLOSE_BRACE;
            CREATE_NEXT(curr);
            continue;
        }

        if(c == '(') {
            curr->type = TOKEN_OPEN_PAREN;
            CREATE_NEXT(curr);
            continue;
        }
        
        if(c == ')') {
            curr->type = TOKEN_CLOSE_PAREN;
            CREATE_NEXT(curr);
            continue;
        }

        if(c == ';') {
            curr->type = TOKEN_SEMICOLON;
            CREATE_NEXT(curr);
            continue;
        }

        if(c == '=') {
            if(n == '=') {
                curr->type = TOKEN_OPERATOR;
                curr->operator_type = OPERATOR_EQUALS;
                i++;
            } else {
                curr->type = TOKEN_ASSIGN;
            }
            CREATE_NEXT(curr);
            continue;
        }

        if(c == '+') {
            curr->type = TOKEN_OPERATOR;
            curr->operator_type = OPERATOR_ADD;
            CREATE_NEXT(curr);
            continue;
        }

        if(c == '-') {
            curr->type = TOKEN_OPERATOR;
            curr->operator_type = OPERATOR_MINUS;
            CREATE_NEXT(curr);
            continue;
        }

        if(c == '*') {
            curr->type = TOKEN_OPERATOR;
            curr->operator_type = OPERATOR_MULT;
            CREATE_NEXT(curr);
            continue;
        }

        if(c == '/') {
            curr->type = TOKEN_OPERATOR;
            curr->operator_type = OPERATOR_DIVID;
            CREATE_NEXT(curr);
            continue;
        }

        if(c == '~') {
            curr->type = TOKEN_OPERATOR;
            curr->operator_type = OPERATOR_BITWISE_COMPLEMENT;
            CREATE_NEXT(curr);
            continue;
        }

        if(c == '!') {
            curr->type = TOKEN_OPERATOR;
            if(n == '=') {
                curr->operator_type = OPERATOR_NOT_EQUAL;
                i++;
            } else
                curr->operator_type = OPERATOR_LOGICAL_NOT;
            CREATE_NEXT(curr);
            continue;
        }

        if(c == '<') {
            curr->type = TOKEN_OPERATOR;
            if(n == '=') {
                curr->operator_type = OPERATOR_LESS_THAN_OR_EQUAL;
                i++;
            } else
                curr->operator_type = OPERATOR_LESS_THAN;
            CREATE_NEXT(curr);
            continue;
        }

        if(c == '>') {
            curr->type = TOKEN_OPERATOR;
            if(n == '=') {
                curr->operator_type = OPERATOR_GREATER_THAN_OR_EQUAL;
                i++;
            } else
                curr->operator_type = OPERATOR_GREATER_THAN;
            CREATE_NEXT(curr);
            continue;
        }

        if(c == '&' && n == '&') {
            curr->type = TOKEN_OPERATOR;
            curr->operator_type = OPERATOR_AND;
            i++;
            CREATE_NEXT(curr);
            continue;
        }

        if(c == '|' && n == '|') {
            curr->type = TOKEN_OPERATOR;
            curr->operator_type = OPERATOR_OR;
            i++;
            CREATE_NEXT(curr);
            continue;
        }

        // look for keywords
        const char* remaining = &content[i];

        if(strncmp("int", remaining, 3) == 0 && (isblank(remaining[3]) || remaining[3] == '\n')) { // TODO: need special strncmp for non-case sensitive compare and for checking stuff after
            curr->type = TOKEN_BUILTIN_TYPE;
            curr->builtin_type = BUILTIN_INT;
            CREATE_NEXT(curr);
            i += 3;
            continue;
        }

        if(strncmp("return", remaining, 6) == 0 && (isblank(remaining[6]) || remaining[6] == '\n')) {
            curr->type = TOKEN_KEYWORD;
            curr->keyword_type = KEYWORD_RETURN;
            CREATE_NEXT(curr);
            i += 6;
            continue;
        }

        // identifiers and literals
        if(isdigit(c)) {
            // create non nested loop for this
            size_t j = i;
            int num = 0;
            for(; j < len; ++j) {
                if(!isdigit(content[j])) {
                    break;
                }

                num *= 10;
                num += content[j] - '0';
            }
            
            curr->type = TOKEN_LITERAL;
            curr->literal_value = num;
            CREATE_NEXT(curr);
            i = j - 1;
            continue;
        }

        if(isalpha(c)) {
            // create non nested loop for this
            size_t j = i;
            for(; j < len; ++j) {
                if(!isalnum(content[j])) {
                    break;
                }
            }

            curr->type = TOKEN_IDENTIFIER;
            curr->id_index = id_arr_find_or_create_substr_i(list->ids, BUILTIN_UNKNOWN, &content[i], j-i);
            CREATE_NEXT(curr);
            i = j-1;
            continue;
        }

        goto fail;
    }

    if(curr->type == TOKEN_INVALID_TOKEN) {
        curr->prev->next = NULL;
        free(curr);
    }

    return list;

fail:
    token_list_free(list);
    return NULL;
}

void token_list_free(token_list_t* list) {
    if(list) {
        token_t* curr = list->head;
        token_t* next;
        while(curr) {
            next = curr->next;
            free(curr);
            curr = next;
        }

        free_id_arr(list->ids);

        free(list);
    }
}

void token_list_print(token_list_t* list) {
    id_arr_debug_print(list->ids);

    token_t* curr = list->head;
    while(curr) {
        printf("%s ", token_type_names[curr->type]);
        switch(curr->type) {
        case TOKEN_IDENTIFIER:
            printf("\"%s\"\n", id_arr_get_name(list->ids, curr->id_index));
            break;
        case TOKEN_LITERAL:
            printf("%u\n", curr->literal_value);
            break;
        case TOKEN_KEYWORD:
            printf("%s\n", keyword_type_names[curr->builtin_type]);
            break;
        case TOKEN_OPERATOR:
            printf("%s\n", operator_type_names[curr->operator_type]);
            break;
        case TOKEN_BUILTIN_TYPE:
            printf("%s\n", builtin_type_names[curr->builtin_type]);
            break;
        default:
            printf("\n");
            break;
        }
        curr = curr->next;
    }
}
