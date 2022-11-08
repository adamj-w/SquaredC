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

token_t* lex(const char* content, size_t len) {
    assert(len > 0);
    assert(content);

    token_t* head;
    ZMALLOC(token_t, head);
    token_t* curr = head;
    for(size_t i = 0; i < len; ++i) {
        char c = content[i];
        if(c == '\0') break;

        if(c == ' ' || c == '\n') continue; // end current token

        // look for structural elements

        if(c == '{') {
            curr->type = TOKEN_OPEN_BRACE;
            ZMALLOC(token_t, curr->next);
            curr->next->prev = curr;
            curr = curr->next;
            continue;
        }
        
        if(c == '}') {
            curr->type = TOKEN_CLOSE_BRACE;
            ZMALLOC(token_t, curr->next);
            curr->next->prev = curr;
            curr = curr->next;
            continue;
        }

        if(c == '(') {
            curr->type = TOKEN_OPEN_PAREN;
            ZMALLOC(token_t, curr->next);
            curr->next->prev = curr;
            curr = curr->next;
            continue;
        }
        
        if(c == ')') {
            curr->type = TOKEN_CLOSE_PAREN;
            ZMALLOC(token_t, curr->next);
            curr->next->prev = curr;
            curr = curr->next;
            continue;
        }

        if(c == ';') {
            curr->type = TOKEN_SEMICOLON;
            ZMALLOC(token_t, curr->next);
            curr->next->prev = curr;
            curr = curr->next;
            continue;
        }

        if(c == '+') {
            curr->type = TOKEN_OPERATOR;
            curr->operator_type = OPERATOR_ADD;
            ZMALLOC(token_t, curr->next);
            curr->next->prev = curr;
            curr = curr->next;
            continue;
        }

        if(c == '-') {
            curr->type = TOKEN_OPERATOR;
            curr->operator_type = OPERATOR_MINUS;
            ZMALLOC(token_t, curr->next);
            curr->next->prev = curr;
            curr = curr->next;
            continue;
        }

        if(c == '*') {
            curr->type = TOKEN_OPERATOR;
            curr->operator_type = OPERATOR_MULT;
            ZMALLOC(token_t, curr->next);
            curr->next->prev = curr;
            curr = curr->next;
            continue;
        }

        if(c == '/') {
            curr->type = TOKEN_OPERATOR;
            curr->operator_type = OPERATOR_DIVID;
            ZMALLOC(token_t, curr->next);
            curr->next->prev = curr;
            curr = curr->next;
            continue;
        }

        if(c == '~') {
            curr->type = TOKEN_OPERATOR;
            curr->operator_type = OPERATOR_BITWISE_COMPLEMENT;
            ZMALLOC(token_t, curr->next);
            curr->next->prev = curr;
            curr = curr->next;
            continue;
        }

        if(c == '!') {
            curr->type = TOKEN_OPERATOR;
            curr->operator_type = OPERATOR_LOGICAL_NOT;
            ZMALLOC(token_t, curr->next);
            curr->next->prev = curr;
            curr = curr->next;
            continue;
        }

        // look for keywords
        const char* remaining = &content[i];

        if(strncmp("int", remaining, 3) == 0 && (isblank(remaining[3]) || remaining[3] == '\n')) { // TODO: need special strncmp for non-case sensitive compare and for checking stuff after
            curr->type = TOKEN_BUILTIN_TYPE;
            curr->builtin_type = BUILTIN_INT;
            ZMALLOC(token_t, curr->next);
            curr->next->prev = curr;
            curr = curr->next;
            i += 3;
            continue;
        }

        if(strncmp("return", remaining, 6) == 0 && (isblank(remaining[6]) || remaining[6] == '\n')) {
            curr->type = TOKEN_KEYWORD;
            curr->keyword_type = KEYWORD_RETURN;
            ZMALLOC(token_t, curr->next);
            curr->next->prev = curr;
            curr = curr->next;
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
            ZMALLOC(token_t, curr->next);
            curr->next->prev = curr;
            curr = curr->next;
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
            curr->name = (char*)malloc(j - i + 1); // USE LESS MALLOC (create cache)
            curr->name_owner = 1;
            strncpy(curr->name, &content[i], j - i);
            curr->name[j-i] = '\0';
            ZMALLOC(token_t, curr->next);
            curr->next->prev = curr;
            curr = curr->next;
            i = j-1;
            continue;
        }

        goto fail;
    }

    if(curr->type == TOKEN_INVALID_TOKEN) {
        curr->prev->next = NULL;
        free(curr);
    }

    return head;

fail:
    free_token_list(head);
    return NULL;
}

void free_token_list(token_t* head) {
    token_t* curr = head;
    token_t* next;
    while(curr) {
        next = curr->next;
        if(curr->type == TOKEN_IDENTIFIER) {
            if(curr->name_owner && curr->name) free(curr->name);
        }
        free(curr);
        curr = next;
    }
}

void debug_print_list(token_t* head) {
    token_t* curr = head;
    while(curr) {
        printf("%s ", token_type_names[curr->type]);
        switch(curr->type) {
        case TOKEN_IDENTIFIER:
            printf("\"%s\"\n", curr->name);
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
