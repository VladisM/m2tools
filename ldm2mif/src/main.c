#include <stdio.h>
#include <string.h>

#include "error.h"
#include "relocator.h"
#include "ldm2mif.h"
#include "args.h"
#include "help.h"

#include <ldm.h>
#include <mif.h>

ldm_buffer_item_t * ldm_buffer;
mif_buffer_item_t * mif_buffer;

static void init_defaults(void);
static void clean_mem(void);

static void init_defaults(void){
    strcpy(settings.out_file, "out.mif");
    settings.file[0] = '\0';
    settings.base_adress = 0x000000;
    settings.size = 8;
    settings.help = 0;
    settings.version = 0;
}

static void clean_mem(void){
    free_ldm_buffer(ldm_buffer);
    free_mif_buffer(mif_buffer);
}

int main(int argc, char **argv){
    init_defaults();
    if(get_args(argc, argv)){
        if(settings.help == 1){
            print_help();
            goto exit;
        }
        else if(settings.version == 1){
            print_version();
            goto exit;
        }
        else{
            goto error_and_exit;
        }
    }

    if( ldm_load(settings.file, &ldm_buffer)              ) goto error_and_exit;
    if( relocate_buffer(ldm_buffer, settings.base_adress) ) goto error_and_exit;
    if( ldm2mif(ldm_buffer, &mif_buffer, settings.size)   ) goto error_and_exit;
    if( mif_write(settings.out_file, mif_buffer)          ) goto error_and_exit;

    goto exit;

error_and_exit:
    print_error();
    clean_mem();
    return -1;

exit:
    clean_mem();
    return 0;
}
