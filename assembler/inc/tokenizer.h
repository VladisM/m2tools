#ifndef TOKENIZER_H_included
#define TOKENIZER_H_included

#include <stdlib.h>
#include <stdint.h>

#include <isa.h>

//these flags determine valid member of union in tok_t
#define FLAGS_IS_LABEL 0x00000001U
#define FLAGS_IS_PSEUD 0x00000002U
#define FLAGS_IS_INSTR 0x00000004U

typedef enum{
    TOKEN_IS_LABEL,
    TOKEN_IS_PSEUD,
    TOKEN_IS_INSTR
}token_type_t;

typedef struct{
    char *line;
}label_t;

typedef struct{
    char * line;
}pseudo_t;

//track files
typedef struct fileInfoOut_s{
    struct fileInfoOut_s *next;
    struct fileInfoOut_s *prev;
    char *absName;
    char *absPath;
    char *name;
}fileInfoOut_t;

//all items parsed by the tokenizer are stored in this struct
typedef struct tok_s{
    struct tok_s *next;                 //it is double linked list :)
    struct tok_s *prev;
    fileInfoOut_t *fileInfo;     //pointer to fileinfo where informations about file are stored
    unsigned int lineNumber;            //token was found on this line
    token_type_t type;
    union{
        label_t *l;
        pseudo_t *p;
        tInstruction *i;
    }payload;
}tok_t;

extern tok_t *toklist_first;
extern tok_t *toklist_last;

void tokenizer(char * filename);
void tokenizer_cleanup(void);

#ifdef DEBUG
void print_filelist(void);
void print_toklist(void);
void print_defs(void);
void print_cons(void);
#endif

#endif
