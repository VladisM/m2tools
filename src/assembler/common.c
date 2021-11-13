#include "common.h"

#include "preprocessor.h"

#include <utillib/core.h>
#include <utillib/utils.h>

error_t *error_buffer = NULL;

void error_buffer_append_if_defined(preprocessed_token_t *tok){
    CHECK_NULL_ARGUMENT(tok);
    if(tok->preprocessed == true){
        ERROR_WRITE("Defined at %s+%ld.", tok->defined.filename, tok->defined.line_number);
    }
}
