#ifndef OBJ_H_included
#define OBJ_H_included

/**
 * @file obj.h
 *
 * @brief Library for dealing with task related with object files.
 */

//TODO Dopsat pořádné komentáře
//TODO Napsat example použití do dokumentace

/**
 * @defgroup obj Object files library
 *
 * @addtogroup obj
 *
 * @{
 */

#include <stdint.h>
#include <isa.h>

typedef enum{
    SYMBOL_EXPORT = 0,
    SYMBOL_IMPORT
}symbol_type_t;

typedef enum{
    DATA_IS_BLOB = 0,
    DATA_IS_INST
}data_symbol_type_t;

typedef struct{
    unsigned int lenght;
    uint8_t *payload;
}datablob_t;

typedef struct data_symbol_s{
    struct data_symbol_s *next;
    struct data_symbol_s *prev;
    data_symbol_type_t type;
    uint32_t address;
    union{
        datablob_t *blob;
        tInstruction *inst;
    }payload;
    uint8_t relocation;
    uint8_t special;
}data_symbol_t;

typedef struct spec_symbol_s{
    char *name;
    struct spec_symbol_s *next;
    struct spec_symbol_s *prev;
    uint32_t value;
    symbol_type_t type;
}spec_symbol_t;

typedef struct section_s{
    char *section_name;
    struct section_s *next;
    struct section_s *prev;
    spec_symbol_t *spec_symbol_first;
    spec_symbol_t *spec_symbol_last;
    data_symbol_t *data_first;
    data_symbol_t *data_last;
}section_t;

typedef struct obj_file_s{
    char *object_file_name;
    section_t *first_section;
    section_t *last_section;
    struct obj_file_s *next;
    struct obj_file_s *prev;
}obj_file_t;

//výčtový typ pro chyby vzniknouvší v modulu
typedef enum{
    OBJRET_OK = 0,
    OBJRET_BROKEN_OBJ,
    OBJRET_NULL_PTR,
    OBJRET_MALLOC_FAIL,
    OBJRET_OBJ_EXIST_ALREADY,
    OBJRET_BROKEN_SECTION,
    OBJRET_FOPEN_ERROR,
    OBJRET_INTERNAL_ERR,
    OBJRET_BROKEN_FILE,
    OBJRET_SECTION_EXIST_ALREADY
}obj_file_err_t;

//funkce pro práci s chybama v modulu
void clear_objlib_errno(void);
obj_file_err_t get_objlib_errno(void);

//funkce pro vyčištění dynamické paměti
void free_object_file(obj_file_t *o);

//funkce pro práci se souborem
int obj_load(char *filename, obj_file_t **o);
int obj_write(char *filename, obj_file_t *o);

//vytvoří nové objekty
int new_obj(char * object_file_name, obj_file_t **o);
int new_section(char *secion_name, section_t **s);
int new_spec_symbol(char *name, uint32_t value, symbol_type_t type, spec_symbol_t **s);
int new_data_symbol(uint32_t address, data_symbol_type_t type, void *payload_ptr, data_symbol_t **d);
int new_blob(unsigned int lenght, datablob_t **b);

//přidává do objektového souboru či do sekce
int append_section_to_obj(obj_file_t *o, section_t *s);
int append_spec_symbol_to_section(section_t *section, spec_symbol_t *symbol);
int append_data_symbol_to_section(section_t *section, data_symbol_t *data);

/**
 * @}
 */

#endif
