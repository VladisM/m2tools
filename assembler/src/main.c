#include <stdlib.h>
#include <stdio.h>

#include <tokenizer.h>
#include <pass1.h>
#include <symbol_table.h>


#ifdef DEBUG
static void print_start(int x);
static void print_end(int x);

int main(int argc, char* argv[]){

    atexit(tokenizer_cleanup);
    atexit(pass1_cleanup);
    atexit(symbol_table_cleanup);

    if(argc == 2){
        print_start(0);

        tokenizer(argv[1]);
        print_toklist();
        print_filelist();
        print_cons();
        print_defs();

        print_end(0);
        print_start(1);

        pass1();
        print_symboltable();
        print_pass1_buffer();

        print_end(1);

        exit(EXIT_SUCCESS);
    }
    else{
        fprintf(stderr, "Wrong arguments!\n");
        exit(EXIT_FAILURE);
    }
}

static void print_start(int x){
    printf("\n----* PASS %d START! *----\n", x);
}

static void print_end(int x){
    printf("\n----* PASS %d END! *----\n", x);
}

#endif
