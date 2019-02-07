#include <stdlib.h>
#include <stdio.h>

#include <tokenizer.h>
#include <pass1.h>
#include <symbol_table.h>
#include <pass2.h>
#include <common_defs.h>
#include <util.h>

tok_t *toklist_first = NULL;
tok_t *toklist_last = NULL;
pass_section_t *pass_list_first = NULL;
pass_section_t *pass_list_last = NULL;

//TODO: přidat parsování argumentů
//TODO: přidat generování obj souboru

#ifdef DEBUG
int main(int argc, char* argv[]){

    atexit(tokenizer_cleanup);
    atexit(pass1_cleanup);
    atexit(symbol_table_cleanup);
    atexit(pass2_cleanup);

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
        print_start(2);

        pass2();
        print_symboltable();
        print_pass2_buffer();

        print_end(2);

        exit(EXIT_SUCCESS);
    }
    else{
        fprintf(stderr, "Wrong arguments!\n");
        exit(EXIT_FAILURE);
    }
}
#endif
