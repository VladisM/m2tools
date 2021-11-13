#ifndef PREPROCESSOR_H_included
#define PREPROCESSOR_H_included

#include <stdbool.h>
#include <utillib/core.h>

typedef struct{
    char *token;
    bool preprocessed;
    struct{
        long line_number;
        long column;
        char *filename;
    } origin;
    struct{
        long line_number;
        long column;
        char *filename;
    } defined;
} preprocessed_token_t;

bool preprocessor_run(char *input_file, queue_t **output);
void preprocessor_clear_output(queue_t *queue);

#endif
