#ifndef LDM2MIF_H_included
#define LDM2MIF_H_included

#include <mif.h>
#include <ldm.h>

int ldm2mif(ldm_buffer_item_t* lb, mif_buffer_item_t** mb, unsigned int size_pow);

#endif
