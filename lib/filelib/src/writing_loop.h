#ifndef FILELIB_WRITING_LOOP_H_included
#define FILELIB_WRITING_LOOP_H_included

#include <utillib/utils.h>

typedef bool (writing_loop_t)(void *input, string_t *output);

writing_loop_t writing_loop_ldm;
writing_loop_t writing_loop_obj;
writing_loop_t writing_loop_sl;

#endif
