#ifndef COMMON_H_included
#define COMMON_H_included

#include "preprocessor.h"
#include <utillib/utils.h>

#define ERROR_WRITE(x, ...) error_buffer_write(error_buffer, (x), ##__VA_ARGS__)

extern error_t *error_buffer;

void error_buffer_append_if_defined(preprocessed_token_t *tok);

#endif
