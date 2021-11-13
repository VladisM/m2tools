#ifndef VERBOSE_H_included
#define VERBOSE_H_included

#include "preprocessor.h"
#include <utillib/core.h>

void verbose_print_preprocessor(queue_t *preprocessor_output);
void verbose_print_pass(int pass);
void verbose_print_generate(void);

#endif
