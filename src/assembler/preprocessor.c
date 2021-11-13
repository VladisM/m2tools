#include "preprocessor.h"

#include "preprocessor_symbol_table.h"
#include "common.h"

#include <stdbool.h>
#include <string.h>

#include <utillib/core.h>
#include <utillib/utils.h>

void preprocessor_clear_file_list(void);
static list_t *file_list = NULL;

static inline unsigned int items_left(queue_t *queue, unsigned int pos){
    return queue_count(queue) - pos - 1;
}

static inline bool is_define_a_macro(queue_t *queue, unsigned int pos){
    CHECK_NULL_ARGUMENT(queue);

    if(items_left(queue, pos) == 0)
        return false;

    token_t *next = NULL;

    list_at((list_t *)queue, pos, (void *)&next);

    unsigned long len = strlen(next->token);

    if(next->token[len] == ')')
        return true;
    else
        return false;
}

static inline bool has_define_two_args(queue_t *queue, unsigned int pos){
    CHECK_NULL_ARGUMENT(queue);

    if(items_left(queue, pos) <= 2)
        return false;

    token_t *next = NULL;
    token_t *next_next = NULL;

    list_at((list_t *)queue, pos + 0, (void *)&next);
    list_at((list_t *)queue, pos + 1, (void *)&next_next);

    if(next->line_number == next_next->line_number)
        return true;
    else
        return false;
}

static bool is_token(token_t *token, char *x){
    CHECK_NULL_ARGUMENT(token);
    CHECK_NULL_ARGUMENT(x);

    if(strcmp(token->token, x) == 0)
        return true;
    else
        return false;
}

static inline bool is_preprocessor(token_t *tok){
    CHECK_NULL_ARGUMENT(tok);

    if(tok->token[0] == '#')
        return true;
    else
        return false;
}

static inline bool is_include(token_t *tok){
    return is_token(tok, "#include");
}

static inline bool is_define(token_t *tok){
    return is_token(tok, "#define");
}

static inline bool is_ifdef(token_t *tok){
    return is_token(tok, "#ifdef");
}

static inline bool is_ifndef(token_t *tok){
    return is_token(tok, "#ifndef");
}

static inline bool is_endif(token_t *tok){
    return is_token(tok, "#endif");
}

static token_t *_token_load(queue_t *queue, unsigned int pos){
    CHECK_NULL_ARGUMENT(queue);

    token_t *tmp = NULL;
    list_at((list_t *) queue, pos, (void*)&tmp);
    return tmp;
}

static preprocessed_token_t *new_preprocessed_token(){
    preprocessed_token_t *tmp = (preprocessed_token_t *)dynmem_calloc(1, sizeof(preprocessed_token_t));

    tmp->token = NULL;
    tmp->preprocessed = false;
    tmp->origin.line_number = 0;
    tmp->origin.column = 0;
    tmp->origin.filename = NULL;
    tmp->defined.line_number = 0;
    tmp->defined.column = 0;
    tmp->defined.filename = NULL;

    return tmp;
}

static void preprocessed_token_destroy(preprocessed_token_t *token){
    CHECK_NULL_ARGUMENT(token);

    if(token->token != NULL)
        dynmem_free(token->token);

    dynmem_free(token);
}

static char *handle_filenames(char *input){

    //check for existence (if found return it)
    for(unsigned int i = 0; i < list_count(file_list); i++){
        char *tmp = NULL;
        list_at(file_list, i, (void *)&tmp);

        if(strcmp(tmp, input) == 0){
            return tmp;
        }
    }

    //didn't found anything, make new record
    char *tmp = dynmem_strdup(input);
    list_append(file_list, (void *)&tmp);

    return tmp;
}

static bool _append_into_output(token_t *token, queue_t *output, pst_t *symbol_table){
    preprocessed_token_t *new_token = new_preprocessed_token();

    new_token->token = dynmem_strdup(token->token);

    new_token->origin.filename = handle_filenames(token->filename);
    new_token->origin.column = token->column;
    new_token->origin.line_number = token->line_number;

    if(pst_is_defined(symbol_table, token)){
        token_t *symbol_value = NULL;

        if(!pst_get_constant_value(symbol_table, token, &symbol_value)){
            ERROR_WRITE("Failed to load value of constant '%s' at %s+%ld.", token->token, token->filename, token->line_number);
            preprocessed_token_destroy(new_token);
            return false;
        }

        new_token->preprocessed = true;

        new_token->defined.filename = handle_filenames(symbol_value->filename);
        new_token->defined.column = token->column;
        new_token->defined.line_number = token->line_number;
    }

    queue_append(output, (void *)&new_token);

    return true;
}

static bool _preprocessor_run(char *input_file, queue_t *output, pst_t *symbol_table, list_t *to_be_cleaned){
    tokenizer_t *tokenizer = NULL;
    queue_t *tokenizer_output = NULL;

    tokenizer_init(&tokenizer);

    if(!tokenizer_tokenize_file(tokenizer, input_file)){
        ERROR_WRITE("Failed to read file %s!", input_file);
        return false;
    }

    tokenizer_end(tokenizer, &tokenizer_output);

    bool retVal = true;
    token_t *head = NULL;
    unsigned falseifs = 0;
    unsigned pos = 0;

    while(pos < queue_count(tokenizer_output)){
        head = _token_load(tokenizer_output, pos++);

        if(falseifs > 0){
            if(is_ifdef(head) || is_ifndef(head)){
                falseifs++;
            }

            if(is_endif(head)){
                falseifs--;
            }
        }
        else{
            if(is_preprocessor(head)){
                if(is_include(head)){
                    token_t *arg = _token_load(tokenizer_output, pos++);

                    if(!_preprocessor_run(arg->token, output, symbol_table, to_be_cleaned)){
                        ERROR_WRITE("Failed to preprocess file %s included at %s+%ld!", arg->token, head->filename, head->line_number);
                        retVal = false;
                        break;
                    }
                }
                else if(is_define(head)){
                    if(is_define_a_macro(tokenizer_output, pos)){
                        ERROR_WRITE("Syntax error at %s+%ld!", head->filename, head->line_number);
                        ERROR_WRITE("Macros aren't supported!");
                        retVal = false;
                        break;
                    }

                    token_t *name = NULL;
                    token_t *value = NULL;

                    if(has_define_two_args(tokenizer_output, pos)){
                        name = _token_load(tokenizer_output, pos++);
                        value = _token_load(tokenizer_output, pos++);
                    }
                    else{
                        name = _token_load(tokenizer_output, pos++);
                    }

                    pst_define_constant(symbol_table, name, value);
                }
                else if(is_ifdef(head)){
                    token_t *arg = _token_load(tokenizer_output, pos++);

                    if(!pst_is_defined(symbol_table, arg)){
                        falseifs++;
                    }
                }
                else if(is_ifndef(head)){
                    token_t *arg = _token_load(tokenizer_output, pos++);

                    if(pst_is_defined(symbol_table, arg)){
                        falseifs++;
                    }
                }
                else if(is_endif(head)){
                    //there is nothing interesting to do
                }
                else{
                    ERROR_WRITE("Unknown preprocessor directive %s at %s+%ld!", head->token, input_file, head->line_number);
                    retVal = false;
                    break;
                }
            }
            else{
                if(!_append_into_output(head, output, symbol_table)){
                    retVal = false;
                    break;
                }
            }
        }
    }

    list_append(to_be_cleaned, (void *)&tokenizer_output);
    return retVal;
}

bool preprocessor_run(char *input_file, queue_t **output){
    CHECK_NULL_ARGUMENT(input_file);
    CHECK_NULL_ARGUMENT(output);
    CHECK_NOT_NULL_ARGUMENT(*output);

    pst_t *symbol_table = NULL;
    list_t *to_be_cleaned = NULL; //list holding all pointers to tokenizer output queues

    queue_init(output, sizeof(preprocessed_token_t *));
    list_init(&to_be_cleaned, sizeof(queue_t *));
    pst_init(&symbol_table);

    if(file_list == NULL){
        list_init(&file_list, sizeof(char *));
        atexit_register(&preprocessor_clear_file_list);
    }

    bool retVal = _preprocessor_run(input_file, *output, symbol_table, to_be_cleaned);

    while(list_count(to_be_cleaned) > 0){
        queue_t *tmp = NULL;
        queue_windraw(to_be_cleaned, (void *)&tmp);
        tokenizer_clean_output_queue(tmp);
    }

    queue_destroy(to_be_cleaned);
    pst_destroy(symbol_table);

    return retVal;
}

void preprocessor_clear_output(queue_t *queue){
    CHECK_NULL_ARGUMENT(queue);

    while(queue_count(queue) > 0){
        preprocessed_token_t *tmp = NULL;
        queue_windraw(queue, (void *)&tmp);
        preprocessed_token_destroy(tmp);
    }

    queue_destroy(queue);
}

void preprocessor_clear_file_list(void){
    if(file_list != NULL){
        while(list_count(file_list) > 0){
            char *tmp = NULL;
            list_windraw(file_list, (void *)&tmp);
            dynmem_free(tmp);
        }
        list_destroy(file_list);
    }
}
