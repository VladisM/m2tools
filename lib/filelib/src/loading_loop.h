#ifndef FILELIB_LOADING_LOOP_H_included
#define FILELIB_LOADING_LOOP_H_included

#include <utillib/core.h>

typedef bool (loading_loop_t)(queue_t *input, void **output, char *filename);

loading_loop_t loading_loop_ldm;
loading_loop_t loading_loop_obj;
loading_loop_t loading_loop_sl;

#endif
