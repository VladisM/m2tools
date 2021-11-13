#ifndef FILELIB_SL_H_included
#define FILELIB_SL_H_included

#include "obj.h"

#include <utillib/core.h>

typedef struct{
    char *object_name;
    obj_file_t *object;
}sl_holder_t;

typedef struct{
    char *target_arch_name;
    list_t *objects;
}sl_file_t;

bool sl_load(char *filename, sl_file_t **f);
bool sl_write(sl_file_t *f, char *filename);

void sl_file_new(sl_file_t **f);
void sl_file_destroy(sl_file_t *f);

void sl_holder_new(sl_holder_t **holder, char *object_name, obj_file_t *object);
void sl_holder_destroy(sl_holder_t *holder);
void sl_holder_into_file(sl_file_t *f, sl_holder_t *holder);

#endif
