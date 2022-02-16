#include "_filelib.h"

error_t *filelib_error_buffer = NULL;

void filelib_init(void){
    error_buffer_init(&filelib_error_buffer);
    platformlib_init();
}

void filelib_deinit(void){
    error_buffer_destroy(filelib_error_buffer);
    platformlib_deinit();
    filelib_error_buffer = NULL;
}

char *filelib_error(void){
    return error_buffer_get(filelib_error_buffer);
}

bool is_token(token_t *token, char *x){
    CHECK_NULL_ARGUMENT(token);
    CHECK_NULL_ARGUMENT(x);

    if(strcmp(token->token, x) == 0)
        return true;
    else
        return false;
}

token_t *_token_load(queue_t *queue){
    CHECK_NULL_ARGUMENT(queue);

    if(queue_count(queue) == 0)
        error("Requesting widraw from empty queue!");

    token_t *tmp = NULL;
    queue_windraw(queue, (void *)&tmp);

    return tmp;
}

bool _load_string(string_t *input, char *filename, void **output, check_structure_t *check_structure, loading_loop_t *loading_loop){
    CHECK_NULL_ARGUMENT(filename);
    CHECK_NULL_ARGUMENT(output);
    CHECK_NOT_NULL_ARGUMENT(*output);
    CHECK_NULL_ARGUMENT(check_structure);
    CHECK_NULL_ARGUMENT(loading_loop);
    CHECK_NULL_ARGUMENT(input);

    queue_t *tokenized_string = NULL;
    tokenizer_t *tokenizer = NULL;

    tokenizer_init(&tokenizer);
    tokenizer_tokenize_string(tokenizer, input);
    tokenizer_end(tokenizer, &tokenized_string);

    if(!(*loading_loop)(tokenized_string, output, filename)){
        tokenizer_clean_output_queue(tokenized_string);
        return false;
    }

    tokenizer_clean_output_queue(tokenized_string);
    (*check_structure)((void *)*output);

    return true;
}

bool _load_file(char *filename, void **output, check_structure_t *check_structure, loading_loop_t *loading_loop){
    CHECK_NULL_ARGUMENT(filename);
    CHECK_NULL_ARGUMENT(output);
    CHECK_NOT_NULL_ARGUMENT(*output);
    CHECK_NULL_ARGUMENT(check_structure);
    CHECK_NULL_ARGUMENT(loading_loop);

    queue_t *tokenized_file = NULL;
    tokenizer_t *tokenizer = NULL;

    tokenizer_init(&tokenizer);

    if(!tokenizer_tokenize_file(tokenizer, filename)){
        FILELIB_ERROR_WRITE("Failed to read file '%s'!", filename);
        return false;
    }

    tokenizer_end(tokenizer, &tokenized_file);

    if(!(*loading_loop)(tokenized_file, output, filename)){
        tokenizer_clean_output_queue(tokenized_file);
        return false;
    }

    tokenizer_clean_output_queue(tokenized_file);
    (*check_structure)(*output);

    return true;
}

bool _write_file(char *filename, void *data, check_structure_t *check_structure,  writing_loop_t *writing_loop){
    CHECK_NULL_ARGUMENT(data);
    CHECK_NULL_ARGUMENT(filename);
    CHECK_NULL_ARGUMENT(check_structure);
    CHECK_NULL_ARGUMENT(writing_loop);

    string_t *tmp = NULL;
    FILE *fp = NULL;

    if(!_write_string(&tmp, data, check_structure, writing_loop)){
        return false;
    }

    fp = fopen(filename, "wb");

    if(fp == NULL){
        FILELIB_ERROR_WRITE("Failed to write %s!", filename);
        string_destroy(tmp);
        return false;
    }

    for(unsigned i = 0; string_at(tmp, i) != '\0'; i++){
        fprintf(fp, "%c", string_at(tmp, i));
    }

    string_destroy(tmp);

    fflush(fp);
    fclose(fp);

    return true;
}

bool _write_string(string_t **output, void *data, check_structure_t *check_structure,  writing_loop_t *writing_loop){
    CHECK_NULL_ARGUMENT(data);
    CHECK_NULL_ARGUMENT(output);
    CHECK_NOT_NULL_ARGUMENT(*output);
    CHECK_NULL_ARGUMENT(check_structure);
    CHECK_NULL_ARGUMENT(writing_loop);

    string_init(output);

    (*check_structure)(data);
    if(!(*writing_loop)(data, *output)){
        string_destroy(*output);
        *output = NULL;
        return false;
    }
    return true;
}

void _multiple_record_error(char *token, char *filename, long line_number){
    FILELIB_ERROR_WRITE("Multiple '%s' record at %s+%ld!", token, filename, line_number);
}

void _missing_record_error(char *token, char *filename){
    FILELIB_ERROR_WRITE("File %s is missing '%s' record!", filename, token);
}

void _not_enough_tokens_error(char *token, char *filename, long line_number){
    FILELIB_ERROR_WRITE("Record %s missing some of its arguments at %s+%ld!", token, filename, line_number);
}

void _wrong_records_order_error(char *token_A, char *token_B, char *filename, long line_number){
    FILELIB_ERROR_WRITE("Found record %s before %s. Wrong sequence at %s:%ld!", token_A, token_B, filename, line_number);
}

void _unexpected_record_error(char *token, char *filename, long line_number){
    FILELIB_ERROR_WRITE("Found unexpected record %s at %s+%ld!", token, filename, line_number);
}

void _cant_decode_isa_address_error(char *filename, long line_number){
    FILELIB_ERROR_WRITE("Can't read ISA address at %d:%ld!", filename, line_number);
}

void _cant_decode_isa_word_error(char *filename, long line_number){
    FILELIB_ERROR_WRITE("Can't read ISA word at %d:%ld!", filename, line_number);
}

void _cant_decode_isa_memory_element(char *filename, long line_number){
    FILELIB_ERROR_WRITE("Can't read ISA memory element at %d:%ld!", filename, line_number);
}

void _wrong_architecture_error(char *filename, char *file_arch){
    FILELIB_ERROR_WRITE("Architecture %s of file %s doesn't mach toolchain architecture %s!", file_arch, filename, TARGET_ARCH_NAME);
}

void _unrecognized_record_error(char *filename, long line_number){
    FILELIB_ERROR_WRITE("Unrecognized record at %s+%ld!", filename, line_number);
}

void _set_arch_name(char **arch){
    CHECK_NULL_ARGUMENT(arch);
    CHECK_NOT_NULL_ARGUMENT(*arch);

    *arch = dynmem_strdup(TARGET_ARCH_NAME);
}
