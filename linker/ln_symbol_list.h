/**
 * @file ln_symbol_list.h
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
 */

//How this work? This sould work as cache with symbols.
//
// Jak jsou parsovány jednotlivé sekce, budou přidávány symboly z nich. Toto se provede
// voláním funkce z ln_section_list.c někde ...
//
// Nad listama pak bude naprogramováno prohledávání aby se zjistila
//   - duplicita exportovaných symbolů (zjišťováno při přidávání symbolu do listu)
//   - uzavřenost množiny symbolů (každý importovaný symbol musí být exportován, zjišťování po dokončení sestavování)
//
// Po sestavení a prohledání listů se s jejich použitím identifikují nevyužité sekce - pozor! při jednosouborovém překladu vznikne
// sekce text která bude obsahovat celý program ale nebude nic exportovat - vzít v úvahu start point v lds souboru

#ifndef LN_SYMBOL_LIST_included
#define LN_SYMBOL_LIST_included

#include <stdbool.h>

#include <obj.h>
#include <isa.h>

#include "ln_section_list.h"
#include "ldparser.h"

/**
 * @brief Enum type with error codes.
 */
typedef enum{
    SYMBOLLIST_OK = 0,      /**< @b Everything is fine. */
    SYMBOLLIST_MULTIPLE,    /**< @b Symbol is exported more than once. */
    SYMBOLLIST_OBJLIB_ERROR /**< @b Error is in objlib. */
}ln_symbol_list_errno_t;

/**
 * @brief Type to hold symbols in the list.
 */
typedef struct symbol_holder_s{
    struct symbol_holder_s *next;   /**< @b Next item in list. */
    spec_symbol_t *sym;             /**< @b Pointer to symbol. */
    section_list_item_t *section;   /**< @b Pointer to section. */
}symbol_holder_t;

/**
 * @brief Pointer to list with exported symbols.
 */
extern symbol_holder_t *exported_symbols;

/**
 * @brief Pointer to list imported symbols.
 */
extern symbol_holder_t *imported_symbols;

/**
 * @brief Clear up error variable.
 */
void clear_symbol_list_errno(void);

/**
 * @brief Return error code.
 */
ln_symbol_list_errno_t get_symbol_list_errno(void);

/**
 * @brief Parse all symbols in section.
 *
 * @param section Section to read.
 *
 * @return true if OK
 */
bool parse_symbols(section_list_item_t *section);

/**
 * @brief Parse symbols defined in linker config file.
 *
 * There are some symbols that can be defined in linker file. These symbols are absolute
 * and they are not mean for relocation.
 *
 * @return true if ok
 */
bool parse_linker_symbols(lds_t *lds);

/**
 * @brief Free all dynamic memory allocated by this module. Should be called at end of programm.
 */
void clean_up_symbol_lists(void);

/**
 * @brief Check if symbols are closed set or not.
 *
 * @return true if all imported symobols are also exported somewhere.
 */
bool check_imported_symbols_exist(void);

#ifndef NDEBUG
/**
 * @brief Printout all known symbols in cashe. Used only for debug purposes.
 */
void print_symbols_lists(void);
#endif

#endif
