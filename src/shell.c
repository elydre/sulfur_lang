#include <stdlib.h>
#include <stdio.h>

#include "../include/lexer.h"
#include "../include/memlib.h"
#include "../include/parser.h"
#include "../include/interpreter.h"
#include "../include/func_interpreter.h"
#include "../sulfur_libs/blt_libs/std.h"
#include "../include/sulfur.h"
#include "../include/make_context.h"
#include "../include/make_include.h"

extern void instructions_print(Instruction* code, int code_len);

/* 
    return 0 if code is incomplete
    return 1 if code is good
    return 2 if code is 'q'
*/
int does_code_is_good(char *code) {
   

    int nub = 0;
    int len = strlen(code);
    int is_in_string = 0;

    while (code[len - 1] == ' ' || code[len - 1] == '\n') {
        len--;
    }

    if (len == 0) return 1;

    // check if all '(' are closed
    for (int i = 0; i < len; i++) {
        if (code[i] == '"') {
            int end = -1;
            for(int k = i + 1; k < len; k++) {
                if (code[k] == '"' && code[k - 1] != '\\') {
                    end = k;
                    break;
                }
            }
            if (end == -1)
                return 0;
            i = end;
            continue;
        }
        if (code[i] == '(') nub++;
        if (code[i] == ')') nub--;
    }  
    if (nub != 0) return 0;

    // check if all '{' are closed
    for (int i = 0; i < len; i++) {
        if (code[i] == '"') {
            int end = -1;
            for(int k = i + 1; k < len; k++) {
                if (code[k] == '"' && code[k - 1] != '\\') {
                    end = k;
                    break;
                }
            }
            if (end == -1)
                return 0;
            i = end;
            continue;
        }
        if (code[i] == '{') nub++;
        if (code[i] == '}') nub--;
    }
    if (nub != 0) return 0;

    // check if all '[' are closed
    for (int i = 0; i < len; i++) {
        if (code[i] == '"') {
            int end = -1;
            for(int k = i + 1; k < len; k++) {
                if (code[k] == '"' && code[k - 1] != '\\') {
                    end = k;
                    break;
                }
            }
            if (end == -1)
                return 0;
            i = end;
            continue;
        }
        if (code[i] == '[') nub++;
        if (code[i] == ']') nub--;
    }
    if (nub != 0) return 0;

    if (len == 1 && code[0] == 'q') return 2;
    return 1;
}

extern char IS_SHELL;

int interactive_shell(sulfur_args_t *args) {
    printf("Welcome to sulfur v%s interactive shell!\n", VERSION);
    printf("Type 'q' or destroy your computer to exit\n");

    // init
    init_memory();
    init_stack();
    init_libs("*shell*");
    make_context();
    IS_SHELL = 1;
    if (args->filepath == NULL) {
        args->filepath = "*shell*";
    }

    // main loop
    char *code = calloc(1000, sizeof(char));
    int decl = 0;
    int exit_code;
    while (1) {
        // read line
        fputs("sulfur > ", stdout);
        fflush(stdout);
        while (1) {
            char *line = fgets(code + decl, 1000 - decl, stdin);
            if (!line) {
                exit_code = 3;
                break;
            }
            decl += strlen(line);
            exit_code = does_code_is_good(code);
            if (exit_code) break;
            fputs("       > ", stdout);
            fflush(stdout);
        }


        if (code[0] == '\n' && exit_code != 2) {
            decl = 0;
            continue;
        }
        if (exit_code > 1)
            goto free_mem;
        // tokenize
        Token *l = lexe(code);
        int len = token_len(l);
        l = make_include(l, &len, args->filepath);

        // parse
        int instruction_len = 0;
        Instruction *insts = parse(l, -1, -1, NULL, &instruction_len);

        if (args->show_parse) {
            instructions_print(insts, instruction_len);
        }
        // execute
            execute(insts, "*shell*", instruction_len);

        // free memory
        for(int i = 0; i < len; i++){
            free_tok_val(l[i]);
        }
        free(l);
        free_mem:
        if (exit_code == 2) break;
        if (exit_code == 3) {
            printf("\n");
             break;
        }
        // clean up
        decl = 0;
    }
    call_to_call_and_free();

    // clean memory
    free(code);

    precision = base_precision;
    if (args->show_mem) {
        printf("\n\nMEMORY:%d\n",MEMORY.len);
        for(int i = 0; i < MEMORY.len; i++){
            printf("    %s: ",MEMORY.keys[i]);
            println_prompt(&MEMORY.values[i],1);
        }
        printf("Press enter to quit\n");
        getchar();
    }

    for(int i = 0; i < MEMORY.len; i++){
        free(MEMORY.keys[i]);
        Obj_free_val(MEMORY.values[i]);
    }

    free(MEMORY.keys);
    free(MEMORY.values);
    return 0;
}
