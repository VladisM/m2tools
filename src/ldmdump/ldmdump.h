#ifndef LDMDUMP_H_included
#define LDMDUMP_H_included

#include "mif_backend.h"
#include "ihex_backend.h"

#include <utillib/core.h>
#include <utillib/cli.h>
#include <utillib/utils.h>
#include <utillib/files.h>

#include <filelib.h>
#include <platformlib.h>

#include <stdbool.h>

#define ERROR_WRITE(msg, ...) { \
    error_buffer_write(error_buffer, (msg), ##__VA_ARGS__); \
}

typedef enum {
    ACTION_NONE,
    ACTION_HELP,
    ACTION_VERSION,
    ACTION_LDM2MIF,
    ACTION_LDM2IHEX
}action_t;

typedef struct{
    action_t action;
    char *input_file;
    bool all;
    char *mem_name;
    char *output_file;
    mif_backend_settings_t mif_settings;
    ihex_backend_settings ihex_settings;
}settings_t;

extern error_t *error_buffer;
extern settings_t settings;

#endif
