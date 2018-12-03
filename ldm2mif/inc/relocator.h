#ifndef RELOCATOR_H_included
#define RELOCATOR_H_included

#include <ldm.h>

int relocate_buffer(ldm_buffer_item_t *b, unsigned int base_address);

#endif
