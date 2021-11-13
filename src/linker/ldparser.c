#include "ldparser.h"

#include "common.h"

#include <platformlib.h>
#include <utillib/core.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static lds_t *_new_lds(void);
static mem_t *_new_mem(char *name, isa_address_t size, isa_address_t orig);
static sym_t *_new_sym(char *name, lds_symbol_type_t type, isa_address_t value, string_t *expresion);
static bool _mem_append(lds_t *lds, mem_t *mem);
static bool _sym_append(lds_t *lds, sym_t *sym);
static bool str_convert_wrap(token_t *t, long long *result);
static bool str_to_isa_addr(token_t *t, isa_address_t *result);
static bool _is_token(token_t *token, char *s);
static token_t *_token_load(queue_t *queue, unsigned *index);
static bool _is_there_enough_tokens_left(queue_t *queue, unsigned needed_tokens, token_t *head, unsigned index);
static bool find_mem_by_name(lds_t *lds, char *name, mem_t **mem);
static bool is_section_in_mem(mem_t *mem, char *section);

bool parse_lds(char *path, lds_t **lds){
    bool returnState = true;
    tokenizer_t *tokenizer = NULL;
    queue_t *tokenizer_output = NULL;
    lds_t *my_lds = NULL;

    tokenizer_init(&tokenizer);

    if(!tokenizer_tokenize_file(tokenizer, path)){
        ERROR_WRITE("Failed to tokenize linker script %s!", path);
        tokenizer_end(tokenizer, &tokenizer_output);
        tokenizer_clean_output_queue(tokenizer_output);
        return false;
    }

    tokenizer_end(tokenizer, &tokenizer_output);

    my_lds = _new_lds();

    for(unsigned index = 0; index < queue_count(tokenizer_output);){
        token_t *head = _token_load(tokenizer_output, &index);

        if(_is_token(head, "MEM")){

            if(!_is_there_enough_tokens_left(tokenizer_output, 3, head, index)){
                returnState = false;
                goto _end_loading_while;
            }

            token_t *mem_name_t = _token_load(tokenizer_output, &index);
            token_t *mem_size_t = _token_load(tokenizer_output, &index);
            token_t *mem_orig_t = _token_load(tokenizer_output, &index);

            isa_address_t orig = 0;
            isa_address_t size = 0;

            if((str_to_isa_addr(mem_size_t, &size) == false) || (str_to_isa_addr(mem_orig_t, &orig) == false)){
                ERROR_WRITE("Syntax error in lds file %s!", head->filename);
                returnState = false;
                goto _end_loading_while;
            }

            mem_t *nm = _new_mem(mem_name_t->token, size, orig);
            _mem_append(my_lds, nm);
        }
        else if(_is_token(head, "PUT")){

            if(!_is_there_enough_tokens_left(tokenizer_output, 2, head, index)){
                returnState = false;
                goto _end_loading_while;
            }

            token_t *put_sec_t = _token_load(tokenizer_output, &index);
            token_t *put_mem_t = _token_load(tokenizer_output, &index);

            mem_t *target_mem = NULL;

            if(!find_mem_by_name(my_lds, put_mem_t->token, &target_mem)){
                ERROR_WRITE("Mem %s doesn't exist, can't assign section %s at %s+%ld!", put_mem_t->token, put_sec_t->token, head->filename, head->line_number);
                returnState = false;
                goto _end_loading_while;
            }

            if(is_section_in_mem(target_mem, put_sec_t->token) == true){
                ERROR_WRITE("Section %s is already assigned in memory %s! Syntax error at %s+%ld.", put_sec_t->token, put_mem_t->token, head->filename, head->line_number);
                returnState = false;
                goto _end_loading_while;
            }

            char *s = dynmem_strdup(put_sec_t->token);
            list_append(target_mem->sections, (void *)&s);
        }
        else if(_is_token(head, "SET")){

            if(!_is_there_enough_tokens_left(tokenizer_output, 2, head, index)){
                returnState = false;
                goto _end_loading_while;
            }

            token_t *sym_name_t = _token_load(tokenizer_output, &index);
            token_t *sym_value_t = _token_load(tokenizer_output, &index);

            //check if this is eval symbol or absolute symbol
            if(_is_token(sym_value_t, "EVAL")){
                token_t *eval_head_t = sym_value_t;

                string_t *eval_expresion = NULL;

                while(1){
                    if(queue_count(tokenizer_output) == (index + 1)){
                        ERROR_WRITE("Syntax error. Unexpected end of file evaluation command from %s+%ld!", eval_head_t->filename, eval_head_t->line_number);
                        returnState = false;
                        goto _end_loading_while;
                    }

                    token_t *eval_part = _token_load(tokenizer_output, &index);

                    if(_is_token(eval_part, "ENDEVAL")){
                        break;
                    }
                    else{
                        if(eval_expresion == NULL){
                            string_init_1(&eval_expresion, eval_part->token);
                        }
                        else{
                            string_append(eval_expresion, " ");
                            string_append(eval_expresion, eval_part->token);
                        }
                    }
                }

                sym_t *ns = _new_sym(sym_name_t->token, LDS_SYMBOL_EVAL, 0, eval_expresion);
                _sym_append(my_lds, ns);
            }
            else{
                isa_address_t value = 0;

                if(str_to_isa_addr(sym_value_t, &value) == false){
                    ERROR_WRITE("Syntax error in lds file %s!", head->filename);
                    returnState = false;
                    goto _end_loading_while;
                }

                sym_t *ns = _new_sym(sym_name_t->token, LDS_SYMBOL_ABSOLUTE, value, NULL);
                _sym_append(my_lds, ns);
            }
        }
        else if(_is_token(head, "ENT")){
            if(!_is_there_enough_tokens_left(tokenizer_output, 1, head, index)){
                returnState = false;
                goto _end_loading_while;
            }

            token_t *name_t = _token_load(tokenizer_output, &index);

            if(my_lds->entry_point == NULL){
                my_lds->entry_point = dynmem_strdup(name_t->token);
            }
            else{
                ERROR_WRITE("Multiple use of ENT! Cannot set multiple entry points at %s+%ld.", name_t->filename, name_t->line_number);
                returnState = false;
                goto _end_loading_while;
            }
        }
        else{
            ERROR_WRITE("Syntax error in lds, unknown directive at %s+%ld.", head->filename, head->line_number);
            returnState = false;
            goto _end_loading_while;
        }
    }

    if(my_lds->entry_point == NULL){
        ERROR_WRITE("Missing entry point setting in file %s. Entry point is mandatory.", path);
        returnState = false;
        goto _end_loading_while;
    }

    if(list_count(my_lds->memories) == 0){
        ERROR_WRITE("Linker script from %s doesn't specify any memory. At least one memory have to be declared.", path);
        returnState = false;
        goto _end_loading_while;
    }

_end_loading_while:

    tokenizer_clean_output_queue(tokenizer_output);

    if(returnState == true){
        *lds = my_lds;
        return true;
    }
    else{
        free_lds(my_lds);
        *lds = NULL;
        return false;
    }
}

void free_lds(lds_t *l){
    if(l == NULL){
        return;
    }

    if(l->entry_point != NULL){
        dynmem_free(l->entry_point);
    }

    if(l->memories != NULL){
        for(unsigned int memories_index = 0; memories_index < list_count(l->memories); memories_index++){
            mem_t *head_mem = NULL;
            list_at(l->memories, memories_index, (void *)&head_mem);

            if(head_mem->name != NULL){
                dynmem_free(head_mem->name);
            }

            if(head_mem->sections != NULL){
                for(unsigned int sections_index = 0; sections_index < list_count(head_mem->sections); sections_index++){
                    char *tmp = NULL;
                    list_at(head_mem->sections, sections_index, (void *)&tmp);

                    dynmem_free(tmp);
                }

                list_destroy(head_mem->sections);
            }

            dynmem_free(head_mem);
        }

        list_destroy(l->memories);
    }

    if(l->symbols != NULL){
        for(unsigned int i = 0; i < list_count(l->symbols); i++){
            sym_t *head_sym = NULL;
            list_at(l->symbols, i, (void *)&head_sym);

            if(head_sym->name != NULL){
                dynmem_free(head_sym->name);
            }

            if(head_sym->eval_expresion != NULL){
                string_destroy(head_sym->eval_expresion);
            }

            dynmem_free(head_sym);
        }

        list_destroy(l->symbols);
    }

    dynmem_free(l);

}

static bool str_convert_wrap(token_t *t, long long *result){
    CHECK_NULL_ARGUMENT(t);
    CHECK_NULL_ARGUMENT(result);

    long last_char = strlen(t->token) - 1;
    long long multiplier = 1;
    char *dup_string = NULL;
    bool retVal = false;

    if(isxdigit(t->token[last_char]) == false){
        if(t->token[last_char] == 'k'){
            multiplier = 1024;
        }
        else if(t->token[last_char] == 'M'){
            multiplier = 1024*1024;
        }
        else{
            ERROR_WRITE("Syntax error at %s+%ld, failed to convert %s to number.", t->filename, t->line_number, t->token);
            return false;
        }
    }

    if(multiplier != 1){
        dup_string = dynmem_strdup(t->token);
        dup_string[last_char] = '\0';
    }
    else{
        dup_string = t->token;
    }

    if(is_number(dup_string) == true){
        if(str_to_num(dup_string, result)){
            retVal = true;
        }
        else{
            error("is_number returned true but str_to_num doesn't!");
        }
    }
    else{
        ERROR_WRITE("Syntax error at %s+%ld, failed to convert %s to number.", t->filename, t->line_number, t->token);
        *result = 0;
    }

    if(multiplier != 1){
        *result = *result * multiplier;
        dynmem_free(dup_string);
    }

    return retVal;
}

static bool str_to_isa_addr(token_t *t, isa_address_t *result){
    CHECK_NULL_ARGUMENT(t);
    CHECK_NULL_ARGUMENT(result);

    long long tmp = 0;

    if(!str_convert_wrap(t, &tmp)){
        return false;
    }

    if(can_fit_in(tmp, sizeof(isa_address_t)) == true){
        *result = (isa_address_t)tmp;
        return true;
    }
    else{
        ERROR_WRITE("Converted number from %s+%ld is too large to fit in isa_address_t!", t->filename, t->line_number);
        return false;
    }
}

static bool _is_token(token_t *token, char *s){
    CHECK_NULL_ARGUMENT(token);
    CHECK_NULL_ARGUMENT(s);

    if(strcmp(token->token, s) == 0){
        return true;
    }
    else{
        return false;
    }
}

static token_t *_token_load(queue_t *queue, unsigned *index){
    CHECK_NULL_ARGUMENT(queue);

    if(queue_count(queue) == 0)
        error("Requesting widraw from empty queue!");

    token_t *tmp = NULL;
    list_at((list_t *)queue, *index, (void *)&tmp);
    (*index)++;

    return tmp;
}

static bool _is_there_enough_tokens_left(queue_t *queue, unsigned needed_tokens, token_t *head, unsigned index){
    CHECK_NULL_ARGUMENT(queue);
    CHECK_NULL_ARGUMENT(head);

    if(queue_count(queue) < (needed_tokens + index)){
        ERROR_WRITE("Syntax error, not enough tokens left at %s+%ld.", head->filename, head->line_number);
        return false;
    }

    return true;
}

static bool find_mem_by_name(lds_t *lds, char *name, mem_t **mem){
    CHECK_NULL_ARGUMENT(lds);
    CHECK_NULL_ARGUMENT(name);

    for(unsigned index = 0; index < list_count(lds->memories); index++){
        mem_t *tmp = NULL;
        list_at(lds->memories, index, (void *)&tmp);

        if(strcmp(tmp->name, name) == 0){
            *mem = tmp;
            return true;
        }
    }
    return false;
}

static bool is_section_in_mem(mem_t *mem, char *section){
    CHECK_NULL_ARGUMENT(mem);
    CHECK_NULL_ARGUMENT(section);

    for(unsigned index = 0; index < list_count(mem->sections); index++){
        char *tmp = NULL;
        list_at(mem->sections, index, (void *)&tmp);

        if(strcmp(tmp, section) == 0){
            return true;
        }
    }

    return false;
}

static inline lds_t *_new_lds(void){
    lds_t *tmp = (lds_t *)dynmem_malloc(sizeof(lds_t));

    tmp->memories = NULL;
    tmp->symbols = NULL;
    tmp->entry_point = NULL;

    list_init(&(tmp->memories), sizeof(mem_t *));
    list_init(&(tmp->symbols), sizeof(sym_t *));

    return tmp;
}

static inline mem_t *_new_mem(char *name, isa_address_t size, isa_address_t orig){
    CHECK_NULL_ARGUMENT(name);

    mem_t *tmp = (mem_t *)dynmem_malloc(sizeof(mem_t));

    tmp->name = dynmem_strdup(name);
    tmp->size = size;
    tmp->orig = orig;

    tmp->sections = NULL;

    list_init(&(tmp->sections), sizeof(char *));

    return tmp;
}

static inline sym_t *_new_sym(char *name, lds_symbol_type_t type, isa_address_t value, string_t *expresion){
    CHECK_NULL_ARGUMENT(name);

    sym_t *tmp = (sym_t *)dynmem_malloc(sizeof(sym_t));

    tmp->name = dynmem_strdup(name);
    tmp->type = type;
    tmp->value = value;
    tmp->eval_expresion = NULL;

    if(type == LDS_SYMBOL_EVAL){
        CHECK_NULL_ARGUMENT(expresion);
        tmp->eval_expresion = expresion;
    }

    return tmp;
}

static inline bool _mem_append(lds_t *lds, mem_t *mem){
    CHECK_NULL_ARGUMENT(lds);
    CHECK_NULL_ARGUMENT(mem);

    for(unsigned int i = 0; i < list_count(lds->memories); i++){
        mem_t *head_mem = NULL;
        list_at(lds->memories, i, (void *)&head_mem);

        if(strcmp(head_mem->name, mem->name) == 0){
            ERROR_WRITE("Mem already exist! Mem: %s.", mem->name);
            return false;
        }
    }

    list_append(lds->memories, (void *)&mem);
    return true;
}

static inline bool _sym_append(lds_t *lds, sym_t *sym){
    CHECK_NULL_ARGUMENT(lds);
    CHECK_NULL_ARGUMENT(sym);

    for(unsigned int i = 0; i < list_count(lds->symbols); i++){
        sym_t *head_sym = NULL;
        list_at(lds->symbols, i, (void *)&head_sym);

        if(strcmp(head_sym->name, sym->name) == 0){
            ERROR_WRITE("Symbol from linker script '%s' already defined!", sym->name);
            return false;
        }
    }

    list_append(lds->symbols, (void *)&sym);
    return true;
}

#ifndef NDEBUG
#define IS_LAST_ONE(list, index) (list_count(list) == ((index) + 1))

void print_lds(lds_t *l){

    if(l == NULL){
        printf("LDS: (null)\n");
    }
    else{
        printf("LDS:\n");
        printf("|- Entry point: %s \n", l->entry_point);

        if(l->memories != NULL){
            printf("|- Mem:\n");

            for(unsigned index = 0; index < list_count(l->memories); index++){
                mem_t *head = NULL;
                list_at(l->memories, index, (void *)&head);

                char *size_s = platformlib_write_isa_address(head->size);
                char *orig_s = platformlib_write_isa_address(head->orig);

                if(IS_LAST_ONE(l->memories, index)){
                    printf("|  '- Name: %s\n", head->name);
                    printf("|     |- size: %s\n", size_s);
                    printf("|     |- orig: %s\n", orig_s);

                    if(list_count(head->sections) == 0){
                        printf("|     '- assig sec: (null)\n");
                    }
                    else{
                        printf("|     '- assig sec: %u\n", list_count(head->sections));

                        for(unsigned index_section = 0; index_section < list_count(head->sections); index_section++){
                            char *tmp = NULL;
                            list_at(head->sections, index_section, (void *)&tmp);

                            if(IS_LAST_ONE(head->sections, index_section)){
                                printf("|        '- '%s'\n", tmp);
                            }
                            else{
                                printf("|        |- '%s'\n", tmp);
                            }
                        }
                    }
                }
                else{
                    printf("|  |- Name: %s\n", head->name);
                    printf("|  |  |- size: %s\n", size_s);
                    printf("|  |  |- orig: %s\n", orig_s);

                    if(list_count(head->sections) == 0){
                        printf("|  |  '- assig sec: (null)\n");
                    }
                    else{
                        printf("|  |  '- assig sec: %u\n", list_count(head->sections));

                        for(unsigned index_section = 0; index_section < list_count(head->sections); index_section++){
                            char *tmp = NULL;
                            list_at(head->sections, index_section, (void *)&tmp);

                            if(IS_LAST_ONE(head->sections, index_section)){
                                printf("|  |     '- '%s'\n", tmp);
                            }
                            else{
                                printf("|  |     |- '%s'\n", tmp);
                            }
                        }
                    }
                }

                dynmem_free(size_s);
                dynmem_free(orig_s);
            }
        }
        else{
            printf("|- Mem: (null)\n");
        }

        if(l->symbols != NULL){
            printf("'- Sym:\n");

            for(unsigned index = 0; index < list_count(l->symbols); index++){
                sym_t *head = NULL;
                list_at(l->symbols, index, (void *)&head);

                char *value_s = platformlib_write_isa_address(head->value);

                if(IS_LAST_ONE(l->symbols, index)){
                    printf("   '- Name: %s\n", head->name);
                    printf("      |- type: %s\n", (head->type == LDS_SYMBOL_ABSOLUTE) ? "absolute" : "eval");
                    printf("      |- value: %s\n", value_s);
                    printf("      '- expresion: '%s'\n", (head->eval_expresion == NULL) ? "NULL" : string_get(head->eval_expresion));
                }
                else{
                    printf("   |- Name: %s\n", head->name);
                    printf("   |  |- type: %s\n", (head->type == LDS_SYMBOL_ABSOLUTE) ? "absolute" : "eval");
                    printf("   |  |- value: %s\n", value_s);
                    printf("   |  '- expresion: '%s'\n", (head->eval_expresion == NULL) ? "NULL" : string_get(head->eval_expresion));
                }

                dynmem_free(value_s);
            }
        }
        else{
            printf("'- Sym: (null)\n");
        }
    }

}

#endif
