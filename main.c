#include <stdlib.h>
#include <stdio.h>
#include "include/lexer.h"
#include "include/memlib.h"
#include "include/parser.h"
#include "include/interpreter.h"
#include "include/func_interpreter.h"
#include "sulfur_libs/blt_libs/std.h"

char *VERSION = "2.4";
/*
args:
    *nothing*     - interactive shell
    *filepath*    - execute file
    -m *filepath* - show memory after execution
    -p *filepath* - show parse tree
    -l *filepath* - show tokens after lexing

    order of flags doesn't matter
*/

int show_mem=0;
int show_parse = 0;
int show_lexe = 0;
char *filepath = NULL;

extern ref_count* REFS;
extern int REFS_len;

extern void parse_main_args(int argc, char** argv);
extern void instructions_print(Instruction* code, int code_len);
extern int interactive_shell();

int execute_file() {
    char *text = read_file(filepath);

    if (!text) {
        printf("File not found: %s\n",filepath);
        return 1;
    }
    Token *l = lexe(text);
    int len = token_len(l); 

    if (show_lexe)
        tokens_print(l, "\n");

    int instruction_len = 0;
    Instruction*code = parse(l, -1, -1, NULL, &instruction_len);

    if (show_parse) {
        instructions_print(code, instruction_len);
    }

    // parser copy values of tokens so
    // you can free tokens after parsing 
    
    init_memory();
    init_stack(); 
    init_libs(filepath);  

    execute(code, filepath, instruction_len);

    precision = base_precision;
    if (show_mem) {
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
    for(int i = 0; i < len; i++){
        free_tok_val(l[i]);
    }
    instruction_free_array(code, instruction_len);
    free(l);

    free(MEMORY.keys);
    free(MEMORY.values);
    free(REFS);

    return 0;
}

int main(int argc,char **argv){
    back_slash_to_path(argv[0]);
    parse_main_args(argc, argv);
    
    if (filepath) {
        return execute_file();
    } else {
        return interactive_shell();
    }
}
