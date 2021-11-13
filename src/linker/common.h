#ifndef COMMON_H_included
#define COMMON_H_included

#include <utillib/utils.h>

#include <stdio.h>
#include <stdbool.h>

#include <utillib/core.h>

#define ERROR_WRITE(msg, ...) { \
    error_buffer_write(error_buffer, (msg), ##__VA_ARGS__); \
}

#define LOG_MSG(msg, ...) { \
    if(settings.verbose == true){ \
        printf("LOG: "); \
        printf((msg, ##__VA_ARGS__)); \
        printf("\r\n"); \
    } \
}

typedef enum{
    ACTION_NOT_SPECIFIED = 0,
    ACTION_HELP,
    ACTION_VERSION,
    ACTION_LINK,
    ACTION_PRINT_LDS
} action_t;

typedef struct{
    action_t action;
    bool verbose;
    char *output_filename;
    bool strip_unused;
    struct {
        list_t *input_obj_files;
        list_t *input_sl_files;
        char *linker_script;
    }input;
} settings_t;

extern error_t *error_buffer;
extern settings_t settings;

#endif
