#include "ldm_test.h"

#include "test_common.h"

#include <stdlib.h>
#include <stdio.h>

#include <filelib.h>
#include <utillib/cli.h>

static bool generate_test(ldm_settings_t *settings, int argc, char **argv){
    UNUSED(settings);

    if(!requested_arguments_exact(argc, 1)){
        return false;
    }

    char *filename = argv[0];

    ldm_file_t *file = NULL;
    ldm_memory_t *mem = NULL;
    ldm_item_t *item_1 = NULL;
    ldm_item_t *item_2 = NULL;

    ldm_file_new(&file);
    ldm_mem_new("RAM_0", &mem, 10, 0);
    ldm_item_new(10, 10, &item_1);
    ldm_item_new(11, 20, &item_2);

    ldm_item_into_mem(mem, item_1);
    ldm_item_into_mem(mem, item_2);
    ldm_mem_into_file(file, mem);
    ldm_file_set_entry(file, 10);

    if(!ldm_write(file, filename)){
        printf("%s\r\n", filelib_error());
        return false;
    }

    ldm_file_destroy(file);

    return true;
}

static bool print_test(ldm_settings_t *settings, int argc, char **argv){
    UNUSED(settings);

    if(!requested_arguments_exact(argc, 1)){
        return false;
    }

    char *filename = argv[0];
    ldm_file_t *ldm = NULL;

    if(!ldm_load(filename, &ldm)){
        printf("%s\r\n", filelib_error());
        return false;
    }

    printf("LDM file %s\r\n", filename);
    printf(" |- arch: %s\r\n", ldm->target_arch_name);

    char *entry_point = platformlib_write_isa_address(ldm->entry_point);
    printf(" |- entry: %s\r\n", entry_point);
    dynmem_free(entry_point);
    entry_point = NULL;

    for(unsigned i = 0; i < list_count(ldm->memories); i++){
        ldm_memory_t *head_mem = NULL;
        list_at(ldm->memories, i, (void *)&head_mem);

        char *memory_origin = platformlib_write_isa_address(head_mem->begin_addr);
        char *memory_size = platformlib_write_isa_address(head_mem->size);

        bool last_mem = false;

        if((i + 1) == list_count(ldm->memories)){
            last_mem = true;
        }

        if(last_mem){
            printf(" '- memory %s\r\n", head_mem->memory_name);
            printf("     |- size: %s\r\n", memory_size);
            printf("     |- origin: %s\r\n", memory_origin);
            printf("     '- items (%u)\r\n", list_count(head_mem->items));
        }
        else{
            printf(" |- memory %s\r\n", head_mem->memory_name);
            printf(" |   |- size: %s\r\n", memory_size);
            printf(" |   |- origin: %s\r\n", memory_origin);
            printf(" |   '- items (%u)\r\n", list_count(head_mem->items));
        }

        dynmem_free(memory_origin);
        dynmem_free(memory_size);

        memory_origin = NULL;
        memory_size = NULL;

        for(unsigned j = 0; j < list_count(head_mem->items); j++){
            ldm_item_t *head_item = NULL;
            list_at(head_mem->items, j, (void *)&head_item);

            char *item_address = platformlib_write_isa_address(head_item->address);
            char *item_value = platformlib_write_isa_instruction_word(head_item->word);

            bool last_item = false;

            if((j + 1) == list_count(head_mem->items)){
                last_item = true;
            }

            char c1 = (last_mem == true) ? ' ' : '|';
            char c2 = (last_item == true) ? '\'' : '|';

            printf(" %c       %c- addr: %s value: %s\r\n", c1, c2, item_address, item_value);

            dynmem_free(item_address);
            dynmem_free(item_value);

            item_address = NULL;
            item_value = NULL;
        }
    }

    ldm_file_destroy(ldm);

    return true;
}

static bool load_save_test(ldm_settings_t *settings, int argc, char **argv){
    UNUSED(settings);

    if(!requested_arguments_exact(argc, 2)){
        return false;
    }

    char *filename_input = argv[0];
    char *filename_output = argv[1];

    ldm_file_t *ldm = NULL;

    if(!ldm_load(filename_input, &ldm)){
        printf("%s\r\n", filelib_error());
        return false;
    }

    if(!ldm_write(ldm, filename_output)){
        printf("%s\r\n", filelib_error());
        ldm_file_destroy(ldm);
        return false;
    }

    ldm_file_destroy(ldm);
    return true;
}

void ldm_test_args_init(options_t *args, ldm_settings_t *settings){
    options_append_section(args, "LDM Tests", NULL);
    options_append_flag_2(args, "ldm-generate", "Generate one small ldm file.");
    options_append_flag_2(args, "ldm-print", "Print content of ldm file.");
    options_append_flag_2(args, "ldm-load-save", "Load ldm file and save it again as another file.");

    settings->generate = false;
    settings->print = false;
    settings->load_save = false;
}

void ldm_test_args_parse(options_t *args, ldm_settings_t *settings){
    if(options_is_flag_set(args, "ldm-generate")){
        settings->generate = true;
    }
    if(options_is_flag_set(args, "ldm-print")){
        settings->print = true;
    }
    if(options_is_flag_set(args, "ldm-load-save")){
        settings->load_save = true;
    }
}

bool ldm_test_should_run(ldm_settings_t *settings){
    return (settings->generate || settings->load_save || settings->print);
}

bool ldm_test_run(ldm_settings_t *settings, int argc, char **argv){
    if(settings->generate == true){
        return generate_test(settings, argc, argv);
    }
    else if(settings->print == true){
        return print_test(settings, argc, argv);
    }
    else if(settings->load_save == true){
        return load_save_test(settings, argc, argv);
    }
    else{
        return false;
    }
}
