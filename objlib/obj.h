#ifndef OBJ_H_included
#define OBJ_H_included

/**
 * @file obj.h
 *
 * @brief Library for dealing with task related with object files.
 */

//TODO: Dopsat pořádné komentáře
//TODO: Napsat example použití do dokumentace

/**
 * @defgroup obj Object files library
 *
 * @addtogroup obj
 *
 * @{
 */

#include <stdint.h>
#include <isa.h>

/**
 * @brief Symbol type.
 */
typedef enum{
    SYMBOL_EXPORT = 0,  /**< @brief Symbol is exported from current section. */
    SYMBOL_IMPORT       /**< @brief Symbol is imported into current section. */
}symbol_type_t;

/**
 * @brief Enum type used to hold information about payload in data_symbol_t struct.
 */
typedef enum{
    DATA_IS_BLOB = 0,   /**< @brief Stored payload in blob struct, datablob_t. */
    DATA_IS_INST        /**< @brief Stored payload in instruction struct, tInstruction. */
}data_symbol_type_t;

/**
 * @brief Structure to hold data blob.
 */
typedef struct{
    unsigned int lenght; /**< @brief Hold information about how long payload is. */
    uint8_t *payload;    /**< @brief Pointer to first byte of payload. */
}datablob_t;

/**
 * @brief Structure to hold information about one elementary data symbol.
 *
 * Data symbol can be two types, blob or inst. Inst mean instruction, and is
 * more specified by pointer to tInstruction struct in file isa.h. Blob is
 * binary blob, it simple data payload without any big meaning.
 *
 * All data symbols in section together form a double linked list.
 */
typedef struct data_symbol_s{
    struct data_symbol_s *next;    /**< @brief Pointer to the next symbol in the list. */
    struct data_symbol_s *prev;    /**< @brief Pointer to the previous symbol in the list. */
    data_symbol_type_t type;       /**< @brief Type of this symbol. */
    uint32_t address;              /**< @brief Address of symbol in memory. This isn't absolute addres, it is rather relative to the beggining of the section.*/
    union{
        datablob_t *blob;          /**< @brief Correct pointer when type == DATA_IS_BLOB. */
        tInstruction *inst;        /**< @brief Correct pointer when type == DATA_IS_INST. */
    }payload;                      /**< @brief Union that hold pointer to payload. */
    uint8_t relocation;            /**< @brief Store information about relocation. 0 if should not be relocated, 1 otherwise. */
    uint8_t special;               /**< @brief Store information about argument, if is special symbol. 1 it is, 0 it isn't. */
}data_symbol_t;

/**
 * @brief Structure to hold information about one special symbol.
 *
 * Special symbols are used by linker, when creating executable binary.
 * All symbols tohether form and double linked list. One list for every section
 * of object file. This double linked list is called special symbol table.
 */
typedef struct spec_symbol_s{
    char *name;                     /**< @brief Name of the symbol. */
    struct spec_symbol_s *next;     /**< @brief Pointer to the next symbol in list.  */
    struct spec_symbol_s *prev;     /**< @brief Pointer to the previous symbol in the list. */
    uint32_t value;                 /**< @brief Value for this symbol.*/
    symbol_type_t type;             /**< @brief Type of the symbol, can be exported or imported to the section.*/
}spec_symbol_t;

/**
 * @brief Structure to hold one section of the object file.
 *
 * All section together in object file, form an double linked list. Each section have
 * its own special symbol table, and its own data symbols (can be instructions or
 * blobs).
 *
 * Section have names, they are used by linker for linking them.
 *
 * They can also be section that dosn't have any symbols exported or inported, and
 * they are valid too. In this case both pointers spec_symbol_first and spec_symbol_last
 * are pointing to NULL.
 */
typedef struct section_s{
    char *section_name;                 /**< @brief Name of this section, name is given by command .SECTION <name> in asm file. */
    struct section_s *next;             /**< @brief Pointer to the next section in object file. */
    struct section_s *prev;             /**< @brief Pointer to the previous sections in boject file. */
    spec_symbol_t *spec_symbol_first;   /**< @brief Pointer to the first element of special symbol table. */
    spec_symbol_t *spec_symbol_last;    /**< @brief Pointer to the last element of the special symbol table. */
    data_symbol_t *data_first;          /**< @brief Pointer to the first element of data stream. */
    data_symbol_t *data_last;           /**< @brief Pointer to the last element of data stream. */
}section_t;

typedef struct obj_file_s{
    char *object_file_name;
    char *target_arch_name;
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
    OBJRET_SECTION_EXIST_ALREADY,
    OBJRET_WRONG_ARCH,
    OBJRET_WRONG_ARG
}obj_file_err_t;

//funkce pro práci s chybama v modulu
void clear_objlib_errno(void);
obj_file_err_t get_objlib_errno(void);

//funkce pro vyčištění dynamické paměti
void free_object_file(obj_file_t *o);

//funkce pro práci se souborem
int obj_load_from_string(char *s, obj_file_t **o);
int obj_load_from_file(char *filename, obj_file_t **o);
int obj_write_to_string(char **s, obj_file_t *o);
int obj_write_to_file(char *filename, obj_file_t *o);

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
