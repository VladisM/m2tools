/**
 * @file ldm.h
 *
 * @brief Library for manipulating LDM files.
 *
 * @author Bc. Vladislav Mlejnecký <v.mlejnecky@seznam.cz>
 * @date 30.09.2019
 *
 * @note This file is part of m2tools project.
 */

#ifndef LDM_H_included
#define LDM_H_included

#include <stdbool.h>
#include <isa.h>

/**
 * @defgroup ldm LDM library
 *
 * LDM is file that hold informations about memories and associated content of
 * memory. Each memory (ldm_mem_t) have name, start address and size. Memory
 * containt items (ldm_item_t). Item is storing one piece of information in
 * that memory. File is stored in structure ldm_file_t, and stored is also arch name,
 * file name, and entry point address.
 *
 * @todo add comments
 *
 * @addtogroup ldm
 *
 * @{
 */

/**
 * @brief
 */
typedef struct ldm_item_s{
    struct ldm_item_s *next;     /**< @b */
    struct ldm_item_s *prev;     /**< @b */
    isa_address_t address;       /**< @b */
    isa_instruction_word_t word; /**< @b */
} ldm_item_t;

/**
 * @brief
 */
typedef struct ldm_mem_s{
    struct ldm_mem_s *next;      /**< @b */
    struct ldm_mem_s *prev;      /**< @b */
    ldm_item_t *first_item;      /**< @b */
    ldm_item_t *last_item;       /**< @b */
    char *mem_name;              /**< @b */
    isa_address_t begin_addr;    /**< @b */
    isa_address_t size;          /**< @b */
    //used internally by linker
    isa_address_t last_offset;   /**< @b Used to store last address in memory when putting sections in. */
} ldm_mem_t;

/**
 * @brief
 */
typedef struct{
    ldm_mem_t *first_mem;
    ldm_mem_t *last_mem;
    char *target_arch_name;
    char *ldm_file_name;
    isa_address_t entry_point;
} ldm_file_t;

/**
 * @brief Error codes
 *
 * Error codes are returnet by function get_ldmlib_errno() and can
 * be used to get more information about potential error.
 */
typedef enum{
    LDMERR_OK = 0,              /**< @brief Everything is ok ... have a nice day :) */
    LDMERR_PTR_NOT_NULL,
    LDMERR_NULL_PTR,
    LDMERR_BROKEN_STRUCT,
    LDMERR_FOPEN_ERROR,
    LDMERR_BROKEN_FILE,
    LDMERR_WRONG_ARCH,
    LDMERR_INTERNAL_ERR,
    LDMERR_MALLOC_FAILED        /**< @brief Error when trying to allocate memory from heap. */
} tLdmError;

/**
 * @brief Open and parse ldm file.
 *
 * @param filename Name of the file that should be readed.
 * @param f Pointer to ldm_file_t pointer. Content of file will be readed there.
 *
 * @return true if ok, false if fail
 */
bool ldm_load(char *filename, ldm_file_t **f);

/**
 * @brief Write ldm file.
 *
 * @param filename Name of the file that should be written.
 * @param f Instance of ldm_file_t struct with content of file.
 *
 * @return true if ok, false if fail
 */
bool ldm_write(char *filename, ldm_file_t *f);

/**
 * @brief Clean up instance of ldm_file_t.
 *
 * @param f Pointer to clear.
 *
 */
void free_ldm_file_struct(ldm_file_t *f);

/**
 * @brief Return last error code.
 *
 * @note Errno variable is simple variable, this function
 * will not clean it after reading. If you call this function
 * you have to call clear_ldmlib_errno() too in order to be
 * able catch next error.
 *
 * @return Error code.
 */
tLdmError get_ldmlib_errno(void);

/**
 * @brief Clean errno variable.
 *
 * For more informations please refer get_ldmlib_errno()
 */
void clear_ldmlib_errno(void);

/**
 * @brief Create new structure that can hold LDM file.
 *
 * @param f Pointer to file. Have to be NULL (*f == NULL)
 * @param filename Name of new file.
 *
 * @return True if ok.
 */
bool new_ldm_file(ldm_file_t **f, char *filename);

/**
 * @brief Set address of starting point.
 *
 * LDM file hold information about where is beggining of code. This is mean
 * for use by loader.
 *
 * @param f Pointer fo LDM file struct.
 * @param entry_point Addres of entry point.
 *
 * @return True if ok.
 */
bool set_entry_point(ldm_file_t *f, isa_address_t entry_point);

/**
 * @brief Create new mem item. Represent piece of real memory on the board.
 *
 * @param m Pointer to new mem, have to be NULL. (*m == NULL)
 * @param mem_name Name of new memory eg. "RAM0".
 * @param begin_addr Absolute address where memory start.
 * @param size Size of memory.
 *
 * @return true if ok.
 */
bool new_mem(ldm_mem_t **m, char *mem_name, isa_address_t begin_addr, isa_address_t size);

/**
 * @brief Create new data item in memory.
 *
 * @param i Pointer to new item, have to be NULL. (*i == NULL)
 * @param address Address of item.
 * @param opword Data of item.
 *
 * @return true if ok.
 */
bool new_item(ldm_item_t **i, isa_address_t address, isa_address_t opword);

/**
 * @brief
 *
 * @param i
 * @param m
 *
 * @return
 */
bool append_item_into_mem(ldm_item_t *i, ldm_mem_t *m);

/**
 * @brief
 *
 * @param m
 * @param f
 *
 * @return
 */
bool append_mem_into_file(ldm_mem_t *m, ldm_file_t *f);

/**
 * @}
 */

#endif
