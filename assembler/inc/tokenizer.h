#ifndef TOKENIZER_H_included
#define TOKENIZER_H_included

void tokenizer(char * filename);
void tokenizer_cleanup(void);

#ifdef DEBUG
void print_filelist(void);
void print_toklist(void);
void print_defs(void);
void print_cons(void);
#endif

#endif
