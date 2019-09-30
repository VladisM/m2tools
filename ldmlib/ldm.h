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
 *
 * @addtogroup ldm
 *
 * @{
 */

typedef struct ldm_item_s{
    struct ldm_item_s *next;
    struct ldm_item_s *prev;
    isa_address_t address;
    isa_instruction_word_t word;
} ldm_item_t;

typedef struct ldm_mem_s{
    struct ldm_mem_s *next;
    struct ldm_mem_s *prev;
    ldm_item_t *first_item;
    ldm_item_t *last_item;
    char *mem_name;
    isa_address_t begin_addr;
    isa_address_t size;
} ldm_mem_t;

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
 * @return true if ok, false if fail
 */
bool free_ldm_file_struct(ldm_file_t *f);

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



bool new_ldm_file(ldm_file_t **f, char *filename, isa_address_t entry_point);
bool new_mem(ldm_mem_t **m, char *mem_name, isa_address_t begin_addr, isa_address_t size);
bool new_item(ldm_item_t **i, isa_address_t address, isa_address_t opword);

/**
 * @}
 */

#endif
