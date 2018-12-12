#include <symbol_table.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static symbol_t *symbol_first = NULL;
static symbol_t *symbol_last = NULL;

static int is_equal(symbol_t *x1, symbol_t *x2);

void new_symbol(char *label, uint32_t address, uint8_t stype, tok_t * parent, void *section){
    symbol_t *x = (symbol_t *) malloc( sizeof(symbol_t) );
    char *l = (char *) malloc( sizeof(char) * (strlen(label) + 1) );

    if(x == NULL || l == NULL){
        fprintf(stderr, "Malloc failed!\n");
        exit(EXIT_FAILURE);
    }

    x->label = l;
    x->address = address;
    x->stype = stype;
    x->parent = parent;
    x->prev = NULL;
    x->next = NULL;
    x->section = section;

    strcpy(l, label);

    for(symbol_t *y = symbol_first; y != NULL; y = y->next){
        if(is_equal(y, x) == 1){
            fprintf(stderr, "Syntax error! Multiple symbol definition!\n");
            fprintf(stderr, "Symbol '%s' from file '%s' at line '%d'.\n", x->label, x->parent->fileInfo->name, x->parent->lineNumber);
            fprintf(stderr, "Previous definition is from file '%s' at line '%d'.\n", y->parent->fileInfo->name, y->parent->lineNumber);
            exit(EXIT_FAILURE);
        }
    }

    if(symbol_first == NULL){
        symbol_first = x;
        symbol_last = x;
    }
    else{
        symbol_last->next = x;
        x->prev = symbol_last;
        symbol_last = x;
    }
}

void symbol_table_cleanup(void){
    symbol_t *tmp, *head;

    head = symbol_first;

    while(head != NULL){
        tmp = head;
        head = head->next;
        free(tmp->label);
        free(tmp);
    }
}

static int is_equal(symbol_t *x1, symbol_t *x2){
    if(
        (strcmp(x1->label, x2->label) == 0) &&
        (x1->stype == x2->stype)
    ) return 1;
    else return 0;
}

#ifdef DEBUG
#include <pass1.h>

void print_symboltable(void){
    printf("\nSymbol table: \n");
    if(symbol_first == NULL){
        printf("  - List is empty\n");
    }
    else{
        for(symbol_t *t = symbol_first; t != NULL; t = t->next){
            printf("  - %-30s \t Value: 0x%08X \t Stype: %d \t Section: '%s'\n", t->label, t->address, t->stype, ((pass1_section_t *)(t->section))->section_name);
        }
    }
}
#endif
