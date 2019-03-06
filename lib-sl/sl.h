#ifndef SL_H_included
#define SL_H_included

#include <obj.h>

typedef struct{
    char *library_name;
    char *target_arch_name;
    obj_file_t *first_obj;
    obj_file_t *last_obj;
}static_library_t;

typedef enum{
    SLRET_OK = 0,
    SLRET_NULL_PTR,
    SLRET_BROKEN_SL,
    SLRET_OBJ_EXIST_ALREADY,
    SLRET_MALLOC_FAIL,
    SLRET_INTERN_ERR,
    SLRET_LIB_EXIST_ALREADY,
    SLRET_CANT_CREATE_SL,
    SLRET_TMP_WRITE_ERR,
    SLRET_CANT_OPEN_SL,
    SLRET_WRONG_ARCH
}sl_err_t;

//handling errors
void clear_sllib_errno(void);
sl_err_t get_sllib_errno(void);

//delete instance of library
void free_sl(static_library_t *lib);

//load and write library files
int sl_load(char *filename, static_library_t **lib);
int sl_write(char *filename, static_library_t *lib);

//create new instace of library
int new_sl(char *lib_name, static_library_t **lib);

//add various object into library instance
int append_objfile_to_sl(obj_file_t *o, static_library_t *lib);

#endif
