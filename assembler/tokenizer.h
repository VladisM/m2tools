/**
 * @file tokenizer.h
 *
 * @brief Two pass assembler with simple preprocessor.
 *
 * @author Bc. Vladislav Mlejnecký <v.mlejnecky@seznam.cz>
 * @date 15.06.2019
 *
 * @note This file is part of m2tools project.
 *
 * Assembler is splitted into multiple files:
 *  - assembler.c
 *  - common_defs.h
 *  - file_gen.c
 *  - file_gen.h
 *  - pass1.c
 *  - pass1.h
 *  - pass2.c
 *  - pass2.h
 *  - symbol_table.c
 *  - symbol_table.h
 *  - tokenizer.c
 *  - tokenizer.h
 *  - util.c
 *  - util.h
 */

#ifndef TOKENIZER_H_included
#define TOKENIZER_H_included

void tokenizer(char * filename);
void tokenizer_cleanup(void);

#ifndef NDEBUG
void print_filelist(void);
void print_toklist(void);
void print_defs(void);
void print_cons(void);
#endif

#endif
