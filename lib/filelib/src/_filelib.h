#ifndef FILELIB_PRIVATE_FILELIB_H_included
#define FILELIB_PRIVATE_FILELIB_H_included

#include "obj.h"
#include "ldm.h"
#include "sl.h"
#include "mif.h"

#include "_obj.h"

#include "struct_check.h"
#include "writing_loop.h"
#include "loading_loop.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <utillib/core.h>
#include <utillib/utils.h>

#include <platformlib.h>
#include <cwalk.h>

#define FILELIB_ERROR_WRITE(x, ...) error_buffer_write(filelib_error_buffer, (x), ##__VA_ARGS__)
#define UNUSED(x) (void)x

extern error_t *filelib_error_buffer;

// simplify parsing of output queue
bool is_token(token_t *token, char *x);
token_t *_token_load(queue_t *queue);

//simplify loading files
bool _load_string(string_t *input, char *filename, void **output, check_structure_t *check_structure, loading_loop_t *loading_loop);
bool _load_file(char *filename, void **output, check_structure_t *check_structure, loading_loop_t *loading_loop);

//simplify writing files
bool _write_file(char *filename, void *data, check_structure_t *check_structure,  writing_loop_t *writing_loop);
bool _write_string(string_t **output, void *data, check_structure_t *check_structure,  writing_loop_t *writing_loop);

//some common error msgs
void _multiple_record_error(char *token, char *filename, long line_number);
void _missing_record_error(char *token, char *filename);
void _not_enough_tokens_error(char *token, char *filename, long line_number);
void _wrong_records_order_error(char *token_A, char *token_B, char *filename, long line_number);
void _unexpected_record_error(char *token, char *filename, long line_number);
void _cant_decode_isa_address_error(char *filename, long line_number);
void _cant_decode_isa_word_error(char *filename, long line_number);
void _cant_decode_isa_memory_element(char *filename, long line_number);
void _wrong_architecture_error(char *filename, char *file_arch);
void _unrecognized_record_error(char *filename, long line_number);

//arch name for structures
void _set_arch_name(char **);

#endif
