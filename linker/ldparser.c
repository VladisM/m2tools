#include "ldparser.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <isa.h>

typedef struct tok_s{
    struct tok_s *next;
    char *tok;
    unsigned int line;
}tok_t;

typedef enum{
    PS_IDLE = 0,
    PS_COMMENT,
    PS_TOK
}lds_parser_state_t;

static inline lds_t *new_lds(void);
static inline mem_t *new_mem(char *name, isa_address_t size, isa_address_t orig);
static inline sym_t *new_sym(char *name, isa_address_t value);
static inline void check_malloc(void *p);
static inline tok_t *new_tok(char *t);

lds_t *parse_lds(char *path){
    FILE *f = fopen(path, "r");
    int c;
    lds_parser_state_t parser_state = PS_IDLE;
    unsigned int line_num = 1;
    char *tok = NULL;
    unsigned int tok_size_real = 2;
    unsigned int tok_size_used = 0;
    lds_t *my_lds = new_lds();
    tok_t *tokens = NULL;
    tok_t *last_tok = NULL;

    tok = malloc(sizeof(char) * tok_size_real);
    check_malloc((void *)tok);

    memset((void *)tok, 0, sizeof(char) * tok_size_real);

    if(f == NULL){
        fprintf(stderr, "Cannot open linker script file '%s'.\n", path);
        free(tok);
        free_lds(my_lds);
        exit(EXIT_FAILURE);
    }

    while((c = fgetc(f)) != EOF){
        if(c == '\r') continue;
        if(c == '\t') c = ' ';
        if(c == '\n') line_num++;

        switch(parser_state){
            case PS_IDLE:
                if(c == ';'){
                    parser_state = PS_COMMENT;
                }
                else if(c == ' ' || c == '\n'){
                    parser_state = PS_IDLE;
                }
                else if(isalnum(c) || c == '.' || c == '_'){
                    goto care_about_token;
                }
                else{
                    fprintf(stderr, "Syntax err in linker script file at line %d.\n", line_num);
                    fclose(f);
                    free(tok);
                    free_lds(my_lds);
                    exit(EXIT_FAILURE);
                }
                break;
            case PS_TOK:
                if(c == '\n' || c == ' '){
                    parser_state = PS_IDLE;

                    if(tok_size_used == tok_size_real){
                        char *new_tok = realloc(tok, sizeof(char) * tok_size_real * 2);
                        check_malloc(new_tok);
                        tok = new_tok;
                        tok_size_real *= 2;
                    }
                    tok[tok_size_used] = '\0';

                    tok_t *new = new_tok(tok);

                    if(tokens == NULL) tokens = new;
                    else last_tok->next = new;

                    last_tok = new;
                    new->line = line_num;

                    memset((void *)tok, 0, sizeof(char) * tok_size_real);
                    tok_size_used = 0;
                }
                else if(isalnum(c) || c == '.' || c == '_'){
care_about_token:
                    parser_state = PS_TOK;

                    if(tok_size_used == tok_size_real){
                        char *new_tok = realloc(tok, sizeof(char) * tok_size_real * 2);
                        check_malloc(new_tok);
                        tok = new_tok;
                        tok_size_real *= 2;
                    }
                    tok[tok_size_used++] = (char)c;
                }
                else{
                    fprintf(stderr, "Syntax error in linker script at line %d.\n", line_num);
                    fclose(f);
                    free(tok);
                    free_lds(my_lds);
                    exit(EXIT_FAILURE);
                }
                break;
            case PS_COMMENT:
                if(c == '\n'){
                    parser_state = PS_IDLE;
                }
                else{
                    parser_state = PS_COMMENT;
                }
                break;
            default:
                fprintf(stderr, "Unknown parser state! Internal error!\n");
                fclose(f);
                free(tok);
                free_lds(my_lds);
                exit(EXIT_FAILURE);
                break;
        }
    }

    fclose(f);
    free(tok);

    for(tok_t *head = tokens; head != NULL; head = head->next){
        //TODO: parse tokens

        if(strcmp(head->tok, "MEM") == 0){

            if(head->next == NULL || head->next->next == NULL || head->next->next->next == NULL){
                fprintf(stderr, "Syntax error in lds file at line %d! Missing argument!\n", head->line);
                exit(EXIT_FAILURE);
            }

            char *mem_name_s = head->next->tok;
            char *mem_size_s = head->next->next->tok;
            char *mem_orig_s = head->next->next->next->tok;

            for(int i = 0; mem_name_s[i] != '\0'; i++){
                if(isalnum(mem_name_s[i]) == 0 && mem_name_s[i] != '.' && mem_name_s[i] != '_'){
                    fprintf(stderr, "Syntax error in lds file at line: %d! Unallowed char in memory name.\n", head->line);
                    exit(EXIT_FAILURE);
                }
            }

            for(int i = 0; mem_size_s[i] != '\0'; i++){
                if(isdigit(mem_size_s[i]) == 0 && mem_size_s[i] != 'k' && mem_size_s[i] != 'M'){
                    fprintf(stderr, "Syntax error in lds file at line: %d! Unallowed char in memory size.\n", head->line);
                    exit(EXIT_FAILURE);
                }
            }

            for(int i = 0; mem_orig_s[i] != '\0'; i++){
                if(isxdigit(mem_orig_s[i]) == 0 && mem_orig_s[i] != 'x'){
                    fprintf(stderr, "Syntax error in lds file at line: %d! Unallowed char in memory origin.\n", head->line);
                    exit(EXIT_FAILURE);
                }
            }

            isa_address_t orig = 0;
            isa_address_t size = 0;
            isa_address_t size_mul = 1;

            char *mem_size_cp_s = (char *) malloc(sizeof(char) * (strlen(mem_size_s) + 1));
            check_malloc(mem_size_cp_s);
            strcpy(mem_size_cp_s, mem_size_s);

            if(sscanf(mem_orig_s, SCNisa_addr, &orig) != 1){
                fprintf(stderr, "Syntax error in lds at line %d! Can't parse orig addr!\n", head->line);
                exit(EXIT_FAILURE);
            }

            for(int i = 0; mem_size_cp_s[i] != '\0'; i++){
                if(mem_size_cp_s[i] == 'k'){
                    size_mul = 1024;
                    mem_size_cp_s[i] = '\0';
                    break;
                }

                if(mem_size_cp_s[i] == 'M'){
                    size_mul = 1024 * 1024;
                    mem_size_cp_s[i] = '\0';
                    break;
                }
            }

            if(sscanf(mem_size_cp_s, "%d", &size) != 1){
                fprintf(stderr, "Syntax error in lds at line %d! Can't parse size!\n", head->line);
                exit(EXIT_FAILURE);
            }

            size *= size_mul;

            mem_t *nm = new_mem(mem_name_s, size, orig);

            for(mem_t *head_mem = my_lds->first_mem; head_mem != NULL; head_mem = head_mem->next){
                if(strcmp(head_mem->name, nm->name) == 0){
                    fprintf(stderr, "Mem already exist! Mem: %s.\n", nm->name);
                    exit(EXIT_FAILURE);
                }
            }

            if(my_lds->first_mem == NULL){
                my_lds->first_mem = nm;
                my_lds->last_mem = nm;
            }
            else{
                my_lds->last_mem->next = nm;
                nm->prev = my_lds->last_mem;
                my_lds->last_mem = nm;
            }

            free(mem_size_cp_s);
            head = head->next->next->next;
        }
        else if(strcmp(head->tok, "PUT") == 0){

            if(head->next == NULL || head->next->next == NULL){
                fprintf(stderr, "Syntax error in lds file at line %d! Missing argument!\n", head->line);
                exit(EXIT_FAILURE);
            }

            char *put_sec_s = head->next->tok;
            char *put_mem_s = head->next->next->tok;

            for(int i = 0; put_mem_s[i] != '\0'; i++){
                if(isalnum(put_mem_s[i]) == 0 && put_mem_s[i] != '.' && put_mem_s[i] != '_'){
                    fprintf(stderr, "Syntax error in lds file at line: %d! Unallowed char in mem name.\n", head->line);
                    exit(EXIT_FAILURE);
                }
            }

            for(int i = 0; put_sec_s[i] != '\0'; i++){
                if(isalnum(put_sec_s[i]) == 0 && put_sec_s[i] != '.' && put_sec_s[i] != '_' && put_sec_s[i] != '*'){
                    fprintf(stderr, "Syntax error in lds file at line: %d! Unallowed char in section name.\n", head->line);
                    exit(EXIT_FAILURE);
                }
            }

            mem_t *target_mem = NULL;

            for(mem_t *head_mem = my_lds->first_mem; head_mem != NULL; head_mem = head_mem->next){
                if(strcmp(head_mem->name, put_mem_s) == 0){
                    target_mem = head_mem;
                    break;
                }
            }

            if(target_mem == NULL){
                fprintf(stderr, "Error at line %d! Memory %s was not found!\n", head->line, put_mem_s);
                exit(EXIT_FAILURE);
            }


            for(unsigned int i = 0; i < target_mem->section_count; i++){
                if(strcmp(target_mem->sections[i], put_sec_s) == 0){
                    fprintf(stderr, "Error, section %s is already assigned to memory %s!\n", put_sec_s, put_mem_s);
                    exit(EXIT_FAILURE);
                }
            }

            char *s = (char *)malloc(sizeof(char) * (strlen(put_sec_s) + 1));
            check_malloc(s);
            strcpy(s, put_sec_s);

            char **new_ptr = (char **)realloc(target_mem->sections, sizeof(char **) * (target_mem->section_count + 1));
            check_malloc(new_ptr);
            target_mem->sections = new_ptr;

            target_mem->sections[(target_mem->section_count)++] = s;

            head = head->next->next;
        }
        else if(strcmp(head->tok, "SET") == 0){

            if(head->next == NULL || head->next->next == NULL){
                fprintf(stderr, "Syntax error in lds file at line %d! Missing argument!\n", head->line);
                exit(EXIT_FAILURE);
            }

            char *sym_name_s = head->next->tok;
            char *sym_value_s = head->next->next->tok;

            for(int i = 0; sym_name_s[i] != '\0'; i++){
                if(isalnum(sym_name_s[i]) == 0 && sym_name_s[i] != '.' && sym_name_s[i] != '_'){
                    fprintf(stderr, "Syntax error in lds file at line: %d! Unallowed char in symbol name.\n", head->line);
                    exit(EXIT_FAILURE);
                }
            }

            for(int i = 0; sym_value_s[i] != '\0'; i++){
                if(isxdigit(sym_value_s[i]) == 0 && sym_value_s[i] != 'x'){
                    fprintf(stderr, "Syntax error in lds file at line: %d! Unallowed char in symbol value.\n", head->line);
                    exit(EXIT_FAILURE);
                }
            }

            isa_address_t value = 0;

            if(sscanf(sym_value_s, SCNisa_addr, &value) != 1){
                fprintf(stderr, "Syntax error in lds at line %d! Can't parse symbol value!\n", head->line);
                exit(EXIT_FAILURE);
            }

            sym_t *ns = new_sym(sym_name_s, value);

            if(my_lds->first_sym == NULL){
                my_lds->first_sym = ns;
                my_lds->last_sym = ns;
            }
            else{
                my_lds->last_sym->next = ns;
                ns->prev = my_lds->last_sym;
                my_lds->last_sym = ns;
            }

            head = head->next->next;
        }
        else if(strcmp(head->tok, "ENT") == 0){
            if(head->next == NULL){
                fprintf(stderr, "Syntax error in lds file at line %d! Missing argument!\n", head->line);
                exit(EXIT_FAILURE);
            }

            tok_t *name = head->next;
            char *name_s = name->tok;

            for(int i = 0; name_s[i] != '\0'; i++){
                if(isalnum(name_s[i]) == 0 && name_s[i] != '.' && name_s[i] != '_'){
                    fprintf(stderr, "Syntax error in lds file at line: %d! Unallowed char in name.\n", head->line);
                }
            }

            if(my_lds->entry_point == NULL){
                char *tmp = (char *)malloc(sizeof(char) * (strlen(name_s) + 1));
                check_malloc(tmp);
                strcpy(tmp, name_s);
                my_lds->entry_point = tmp;
            }
            else{
                fprintf(stderr, "Multiple use of ENT! Cannot set multiple entry points!\n");
                exit(EXIT_FAILURE);
            }

            head = head->next;
        }
        else{
            fprintf(stderr, "Syntax error in lds file at line: %d!\n", head->line);
            exit(EXIT_FAILURE);
        }
    }

    {
        tok_t *tmp = NULL;
        tok_t *head = tokens;

        while(head != NULL){
            tmp = head;
            head = head->next;

            free(tmp->tok);
            free(tmp);
        }
    }

    return my_lds;
}

void free_lds(lds_t *l){
    if(l != NULL){
        if(l->entry_point != NULL){
            free(l->entry_point);
        }
        if(l->first_mem != NULL){
            mem_t *tmp, *head;

            head = l->first_mem;
            while(head != NULL){
                tmp = head;
                head = head->next;

                for(unsigned int i = 0; i < tmp->section_count; i++){
                    free(tmp->sections[i]);
                }

                free(tmp->name);
                free(tmp->sections);
                free(tmp);
            }
        }
        if(l->first_sym != NULL){
            sym_t *tmp, *head;

            head = l->first_sym;
            while(head != NULL){
                tmp = head;
                head = head->next;

                free(tmp->name);
                free(tmp);
            }
        }
        free(l);
    }
}

static inline lds_t *new_lds(void){
    lds_t *tmp = (lds_t *)malloc(sizeof(lds_t));

    check_malloc((void *)tmp);

    memset(tmp, 0, sizeof(lds_t));

    tmp->first_mem = NULL;
    tmp->last_mem = NULL;
    tmp->entry_point = NULL;
    tmp->first_sym = NULL;
    tmp->last_sym = NULL;

    return tmp;
}

static inline mem_t *new_mem(char *name, isa_address_t size, isa_address_t orig){
    mem_t *tmp = (mem_t *)malloc(sizeof(mem_t));
    char *tmp_s = (char *)malloc(sizeof(char) * (strlen(name) + 1));

    check_malloc((void *)tmp);
    check_malloc((void *)tmp_s);

    memset(tmp, 0, sizeof(mem_t));
    strcpy(tmp_s, name);

    tmp->name = tmp_s;
    tmp->size = size;
    tmp->orig = orig;

    tmp->next = NULL;
    tmp->prev = NULL;
    tmp->section_count = 0;
    tmp->sections = NULL;

    return tmp;
}

static inline sym_t *new_sym(char *name, isa_address_t value){
    sym_t *tmp = (sym_t *)malloc(sizeof(sym_t));
    char *tmp_s = (char *)malloc(sizeof(char) * (strlen(name) + 1));

    check_malloc((void *)tmp);
    check_malloc((void *)tmp_s);

    memset(tmp, 0, sizeof(sym_t));
    strcpy(tmp_s, name);

    tmp->name = tmp_s;
    tmp->value = value;
    tmp->next = NULL;
    tmp->prev = NULL;

    return tmp;
}

static inline void check_malloc(void *p){
    if(p == NULL){
        fprintf(stderr, "Malloc failed!\n");
        exit(EXIT_FAILURE);
    }
}

static inline tok_t *new_tok(char *t){
    tok_t *tmp = (tok_t *)malloc(sizeof(tok_t));
    char *tmp_s = (char *)malloc(sizeof(char) * (strlen(t) + 1));

    check_malloc((void *)tmp);
    check_malloc((void *)tmp_s);

    strcpy(tmp_s, t);
    tmp->tok = tmp_s;
    tmp->next = NULL;

    return tmp;
}

#ifndef NDEBUG
void print_lds(lds_t *l){

    if(l == NULL){
        printf("LDS: (null)\n");
    }
    else{
        printf("LDS:\n");
        printf("|- Entry point: %s \n", l->entry_point);

        if(l->first_mem != NULL){
            printf("|- Mem:\n");

            for(mem_t *head = l->first_mem; head != NULL; head = head->next){
                if(head->next != NULL){
                    printf("|  |- Name: %s\n", head->name);
                    printf("|  |  |- size:"PRIisa_addr"\n", head->size);
                    printf("|  |  |- orig:"PRIisa_addr"\n", head->orig);

                    if(head->section_count == 0){
                        printf("|  |  '- assig sec: (null)\n");
                    }
                    else{
                        printf("|  |  '- assig sec: %d\n", head->section_count);
                        for(unsigned int i = 0; i < head->section_count - 1; i++){
                            printf("|  |     |- '%s'\n", head->sections[i]);
                        }
                        printf("|  |     '- '%s'\n", head->sections[head->section_count - 1]);
                    }
                }
                else{
                    printf("|  '- Name: %s\n", head->name);
                    printf("|     |- size:"PRIisa_addr"\n", head->size);
                    printf("|     '- orig:"PRIisa_addr"\n", head->orig);
                    if(head->section_count == 0){
                        printf("|     '- assig sec: (null)\n");
                    }
                    else{
                        printf("|     '- assig sec: %d\n", head->section_count);
                        for(unsigned int i = 0; i < head->section_count - 1; i++){
                            printf("|        |- '%s'\n", head->sections[i]);
                        }
                        printf("|        '- '%s'\n", head->sections[head->section_count - 1]);
                    }
                }
            }
        }
        else{
            printf("|- Mem: (null)\n");
        }

        if(l->first_sym != NULL){
            printf("'- Sym:\n");

            for(sym_t *head = l->first_sym; head != NULL; head = head->next){
                if(head->next != NULL){
                    printf("   |- Name: %s\n", head->name);
                    printf("   |  '- value:"PRIisa_addr"\n", head->value);
                }
                else{
                    printf("   '- Name: %s\n", head->name);
                    printf("      '- value:"PRIisa_addr"\n", head->value);
                }
            }
        }
        else{
            printf("'- Sym: (null)\n");
        }
    }

}
#endif
