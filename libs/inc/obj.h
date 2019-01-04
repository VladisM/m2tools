#ifndef OBJ_H_included
#define OBJ_H_included

/**
 * @file obj.h
 *
 * @brief Library for dealing with task related with object files.
 *
 *
 *
 * @todo Dopsat pořádné komentáře
 *
 * @todo Napsat example použití do dokumentace
 *
 * @todo přidat podporu pro kontrolu již existujících sekcí v objektu při vkládání - není možno mít dvě sekce stejného jména
 */

/**
 * @defgroup obj Object files library
 *
 * @addtogroup obj
 *
 * @{
 */

#include <stdint.h>

#define FLAG_DATA_RELOCATION 0x01
#define FLAG_DATA_SPECIAL    0x02

typedef enum{
    SYMBOL_EXPORT = 0,
    SYMBOL_IMPORT
}symbol_type_t;

typedef struct data_symbol_s{
    struct data_symbol_s *next;
    struct data_symbol_s *prev;
    uint32_t address;
    uint32_t value;
    uint8_t flags;
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
    OBJRET_BROKEN_FILE
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
int new_data_symbol(uint32_t address, uint32_t value, int relocation, int special, data_symbol_t **d);

//přidává do objektového souboru či do sekce
int append_section_to_obj(obj_file_t *o, section_t *s);
int append_spec_symbol_to_section(section_t *section, spec_symbol_t *symbol);
int append_data_symbol_to_section(section_t *section, data_symbol_t *data);

/**
 * @}
 */

#endif
