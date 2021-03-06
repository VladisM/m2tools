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

#include <isa.h>
#include <obj.h>
#include <ldm.h>
#include <sl.h>

#include "ln_section_list.h"
#include "ldparser.h"

/**
 * @brief Enum type with error codes.
 */
typedef enum{
    SYMBOLLIST_OK = 0,          /**< @b Everything is fine. */
    SYMBOLLIST_MULTIPLE,        /**< @b Symbol is exported more than once. */
    SYMBOLLIST_OBJLIB_ERROR,    /**< @b Error is in objlib. */
    SYMBOLLIST_MISSING_EXPORT,  /**< @b There are unresolved imported symbols. */
    SYMBOLLIST_WRONG_ARG        /**< @b Wrong arguments are given ... NULL pointer? */
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
 * @param entry_point In addition to all symbols, also entry point have to be defined. It is
 * kind of special symbol "imported" by linker script.
 *
 * @return true if all imported symobols are also exported somewhere.
 */
bool check_imported_symbols_exist(char *entry_point);

/**
 * @brief Find section, where symbol is exported and mark it as used. For entry points and so on.
 *
 * @param symbol_name Name of symbol to search for.
 *
 * @return True if ok.
 */
bool mark_section_with_symbol_as_used(char *symbol_name);

/**
 * @brief Search imported symbol cache and find symbol with same value and from same section.
 *
 * @param section We are going to search in this section.
 * @param id Value that symbol have to had. This is immediate operand from instructions marked as special.
 * @param result Pointer to empty pointer. Will be set to point on founded symbol.
 *
 * @return true if ok.
 */
bool search_for_import_symbol_name(section_list_item_t *section, isa_address_t id, symbol_holder_t **result);

/**
 * @brief Search for exported counterpart symbol to given one.
 *
 * @param import_label Pointer to imported symbol to search for.
 * @param result Pointer that will be set to point at founded symbol.
 *
 * @return true if ok.
 */
bool search_for_exported_symbol(symbol_holder_t *import_label, symbol_holder_t **result);

#ifndef NDEBUG
/**
 * @brief Printout all known symbols in cashe. Used only for debug purposes.
 */
void print_symbols_lists(void);
#endif

#endif
