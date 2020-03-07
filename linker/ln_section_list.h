/**
 * @file ln_section_list.h
 *
 * @brief Portable linker from m2tools.
 *
 * @author Bc. Vladislav Mlejnecký <v.mlejnecky@seznam.cz>
 * @date 07.02.2020
 *
 * @note This file is part of m2tools project.
 *
 * Linker is splitted into multiple files:
 *  - ldparser.c
 *  - ldparser.h
 *  - linker.c
 *  - linker_util.c
 *  - linker_util.h
 *  - ln_section_list.c
 *  - ln_section_lish.h
 *  - ln_symbol_list.c
 *  - ln_symbol_list.h
 *
 * This module contain prarts related to creating and holding section list. All
 * futher work with linking is based on this list.
 */

#ifndef LN_SECTION_LIST_included
#define LN_SECTION_LIST_included

#include <stdbool.h>

#include <isa.h>
#include <ldm.h>
#include <obj.h>
#include <sl.h>

/**
 * @brief Enum type that define error codes that can occur in this module.
 */
typedef enum{
    SECTION_OK = 0,                 /**< @b Everything is fine.*/
    SECTION_WRONG_ARG,              /**< @b You passed wrong arch into fuction ... you fool! */
    SECTION_OBJLIB_ERROR,           /**< @b Error occured in underling objlib module. */
    SECTION_ISALIB_ERROR,           /**< @b Error occured in underling isalib module. */
}ln_section_list_errno_t;

/**
 * @brief Structure that hold items in section list.
 */
typedef struct section_list_item_s{
    section_t *section;                 /**< @b Pointer to section as defined by objlib.*/
    ldm_mem_t *assinged_mem;            /**< @b Pointer to assinged mem structure as defined by ldmlib.*/
    struct section_list_item_s *next;   /**< @b Pointer to next item in the list.*/
    isa_address_t begin_addr;           /**< @b Address offset of this section in assigned memory.*/
    bool used;                          /**< @b Flag that specifies if section is used or not.*/
}section_list_item_t;

/**
 * @brief Pointer to list with sections.
 */
extern section_list_item_t *first_section_item;

/**
 * @brief Append whole static library as loaded by sllib into section list.
 *
 * @param sl Pointer to library. Should be loaded from file with sl_load()
 *
 * @return true if OK
 */
bool append_into_section_list_sl(static_library_t *sl);

/**
 * @brief Append specified object file into section list.
 *
 * @param obj Pointer to object file structure as defined in objlib.
 *
 * @return true if OK
 */
bool append_into_section_list_obj(obj_file_t *obj);

/**
 * @brief For working with errno in section list module. This function will clear it.
 */
void clear_section_list_errno(void);

/**
 * @brief Return section list errno.
 *
 * @return One of the ln_section_list_errno_t.
 */
ln_section_list_errno_t get_section_list_errno(void);

/**
 * @brief Free all dynamically allocated memory. Should be called at the end of program.
 */
void clean_up_section_list(void);

#ifndef NDEBUG
/**
 * @brief Print list of all sections, used for debbuging purposes during development.
 */
void print_section_list(void);

/**
 * @brief Print short sum with info about used and unused sections. Used for debugging during development.
 */
void print_section_status(void);
#endif

#endif
