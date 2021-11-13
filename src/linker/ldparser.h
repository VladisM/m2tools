#ifndef LDPARSER_H_included
#define LDPARSER_H_included

#include <stdint.h>

#include <platformlib.h>
#include <utillib/core.h>

typedef enum{
    LDS_SYMBOL_ABSOLUTE,
    LDS_SYMBOL_EVAL
}lds_symbol_type_t;

typedef struct{
    char *name;
    isa_address_t size;
    isa_address_t orig;
    list_t *sections;   //type char *
}mem_t;

typedef struct{
    char *name;
    lds_symbol_type_t type;
    isa_address_t value;
    string_t *eval_expresion;
}sym_t;

typedef struct lds_s{
    list_t *memories; //type mem_t *
    list_t *symbols; //type sym_t *
    char *entry_point;
}lds_t;

bool parse_lds(char *path, lds_t **lds);
void free_lds(lds_t *l);

#ifndef NDEBUG
void print_lds(lds_t *l);
#endif

#endif
