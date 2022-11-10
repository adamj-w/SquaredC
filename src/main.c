/*
 * Created on Sat Nov 05 2022
 *
 * Copyright (c) 2022 Adam Warren
 */
#include "lexer.h"
#include "parser.h"
#include "nodes.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "asm_gen.h"

#include <assert.h>

static int verbose;

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: %s [file]\n", argv[0]);
        exit(-1);
    }

    verbose = 0;
    if(argc >= 3) {
        if(strcmp(argv[2], "-vv") == 0)
            verbose = 2;
        else if(strcmp(argv[2], "-v") == 0)
            verbose = 1;
    }

    FILE* fp;
    fp = fopen(argv[1], "r");
    if(!fp || errno != 0) {
        printf("Failed to open file at %s\n", argv[1]);
        exit(-1);
    }

    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if(len == 0) {
fail:
        printf("Failed\n");
        fclose(fp);
        exit(0);
    }

    char* data = (char*)malloc(len + 1);
    if(!data) { goto fail; }

    if(fread(data, 1, len, fp) != len) { goto fail; }
    data[len] = '\0';

    fclose(fp);

    if(verbose > 1) printf("\n%s\n\n\n", &data[0]);

    token_t* tokens = lex(data, len);
    free(data);

    if(!tokens) { 
        printf("Failed to tokenize file\n"); 
        exit(-1);
    }
    if(verbose) { 
        debug_print_list(tokens);
        printf("\n\n");
    }

    node_root_t* root = parse(tokens);
    free_token_list(tokens);
    if(!root) {
        printf("Failed to parse file\n");
        exit(-1);
    }

    if(verbose) {debug_print_node_tree(root);}

    argv[1][strlen(argv[1]) - 2] = '\0';

    size_t outfile_len = strlen(argv[1]) + 2;
    char* outassembly = (char*)malloc(outfile_len + 1);
    strncpy(outassembly, argv[1], outfile_len);
    strncat(outassembly, ".s", outfile_len);

    fp = fopen(outassembly, "w+");
    if(!fp || errno) {
        printf("Failed to write assembly intermediate");
        exit(-1);
    }

    bool success = generate_asm(fp, root);

    fclose(fp);

    free_root_node(root);

    if(success) {
        size_t cmd_len = 14 + outfile_len + outfile_len - 3 + 200;
        char* cmd_buf = (char*)malloc(cmd_len + 1);
        snprintf(cmd_buf, cmd_len, "gcc -m32 %s -o %s", outassembly, argv[1]);

        system(cmd_buf);
        free(cmd_buf);

        remove(outassembly);
    }

    free(outassembly);
    exit(0);
}
