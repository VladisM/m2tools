#ifndef TEST_COMMON_H_included
#define TEST_COMMON_H_included

#include <stdbool.h>

#define UNUSED(x) (void)x

bool requested_arguments_exact(int argc, int requested);
bool requested_arguments_more(int argc, int requested);


#endif
