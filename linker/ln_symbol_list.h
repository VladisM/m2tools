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

//TODO: add comments

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

typedef enum{
    SYMBOLLIST_OK = 0,
    SYMBOLLIST_MALLOC,
    SYMBOLLIST_WTF,
    SYMBOLLIST_MULTIPLE
}ln_symbol_list_errno_t;

typedef struct symbol_holder_s{
    struct symbol_holder_s *next;
    spec_symbol_t *sym;
    section_list_item_t *section;
}symbol_holder_t;

extern symbol_holder_t *exported_symbols;
extern symbol_holder_t *imported_symbols;

void clear_symbol_list_errno(void);
ln_symbol_list_errno_t get_symbol_list_errno(void);

bool parse_symbols(section_list_item_t *section);

void clean_up_symbol_lists(void);

#ifndef NDEBUG
void print_symbols_lists(void);
#endif

#endif
