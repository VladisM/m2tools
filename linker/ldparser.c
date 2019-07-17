#include "ldparser.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <isa.h>

static inline lds_t *new_lds(void);
static inline mem_t *new_mem(char *name, isa_address_t size, isa_address_t orig);
static inline sym_t *new_sym(char *name, isa_address_t value);
static inline void check_malloc(void *p);

lds_t *parse_lds(char *path){
    //TODO: add parsing script
    return NULL;
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
                    printf("|  |  |- size:"PRIisa_addr, head->size);
                    printf("|  |  '- orig:"PRIisa_addr, head->orig);
                }
                else{
                    printf("|  |- Name: %s\n", head->name);
                    printf("|     |- size:"PRIisa_addr, head->size);
                    printf("|     '- orig:"PRIisa_addr, head->orig);
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
                    printf("   |  '- value:"PRIisa_addr, head->value);
                }
                else{
                    printf("   '- Name: %s\n", head->name);
                    printf("      '- value:"PRIisa_addr, head->value);
                }
            }
        }
        else{
            printf("'- Sym: (null)\n");
        }
    }

}
#endif
