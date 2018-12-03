#include <stdio.h>
#include <string.h>

#include "error.h"
#include "relocator.h"
#include "sender.h"
#include "args.h"
#include "help.h"

#include <ldm.h>

static void init_default(void);
static void clean_mem(void);

ldm_buffer_item_t * buffer;

static void init_default(void){
    strcpy(settings.file, "none");
    strcpy(settings.port, "none");
    settings.base_adress = 0;
    settings.help = 0;
    settings.version = 0;
}

static void clean_mem(void){
    free_ldm_buffer(buffer);
}

int main(int argc, char **argv)
{
    init_default();
    //get and parse arguments also take care baout --help and --version
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

    if (ldm_load(settings.file, &buffer) < 0)              goto error_and_exit;
    if (relocate_buffer(buffer, settings.base_adress) < 0) goto error_and_exit;

    sender_info_t info;
    info.base_address = settings.base_adress;
    info.port = settings.port;
    info.emulator = settings.emulator;

    if (send_buffer(&info, buffer) < 0) goto error_and_exit;

    goto exit;

error_and_exit:
    print_error();
    clean_mem();
    return -1;

exit:
    clean_mem();
    return 0;

}
