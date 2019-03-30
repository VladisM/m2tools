#ifndef UTIL_H_included
#define UTIL_H_included

#include <stdint.h>

typedef union{
    uint8_t byte;
    uint16_t hword;
    uint32_t word;
}val_t;

typedef enum{
    BYTE,
    HWORD,
    WORD
}val_types_t;

long int convert_to_int(char *l);
int is_number(char *s);
int format_integer(val_types_t size, val_t *out_val, long int val);
char *basename(char *path);

#ifndef NDEBUG
void print_start(int x);
void print_end(int x);
#endif

#endif
