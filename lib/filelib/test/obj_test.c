#include "obj_test.h"

#include "test_common.h"

#include <stdlib.h>
#include <stdio.h>

#include <filelib.h>
#include <utillib/cli.h>

static bool generate_test(obj_settings_t *settings, int argc, char **argv){
    UNUSED(settings);

    if(!requested_arguments_exact(argc, 1)){
        return false;
    }

    char *filename = argv[0];

    obj_file_t *file = NULL;

    obj_section_t *section_A = NULL;
    obj_section_t *section_B = NULL;

    obj_symbol_t *symbol_A = NULL;
    obj_symbol_t *symbol_B = NULL;
    obj_symbol_t *symbol_C = NULL;

    obj_data_t *data_A = NULL;
    obj_data_t *data_B = NULL;
    obj_data_t *data_C = NULL;
    obj_data_t *data_D = NULL;

    obj_data_t *blob_A = NULL;
    obj_data_t *blob_B = NULL;

    obj_file_new(&file);

    obj_section_new("section_A", &section_A);
    obj_section_new("section_B", &section_B);

    obj_symbol_new(&symbol_A, "symbol_A", 0x0A);
    obj_symbol_new(&symbol_B, "symbol_B", 0x0B);
    obj_symbol_new(&symbol_C, "symbol_C", 0x0C);

    obj_data_new(&data_A, 0x00, 0x0A, 0x00, false, false);
    obj_data_new(&data_B, 0x04, 0x0B, 0x00, false, false);
    obj_data_new(&data_C, 0x00, 0x0C, 0x00, false, false);
    obj_data_new(&data_D, 0x04, 0x0D, 0x00, false, false);

    obj_blob_new(&blob_A, 0x05, 0x0A);
    obj_blob_new(&blob_B, 0x05, 0x0B);

    obj_data_into_section(section_A, data_A);
    obj_data_into_section(section_A, data_B);
    obj_blob_into_section(section_A, blob_A);

    obj_data_into_section(section_B, data_C);
    obj_data_into_section(section_B, data_D);
    obj_blob_into_section(section_B, blob_B);

    obj_exported_symbol_into_section(section_A, symbol_A);
    obj_exported_symbol_into_section(section_A, symbol_B);
    obj_imported_symbol_into_section(section_B, symbol_C);

    obj_section_into_file(file, section_A);
    obj_section_into_file(file, section_B);

    if(!obj_write(file, filename)){
        printf("%s\r\n", filelib_error());
        obj_file_destroy(file);
        return false;
    }

    obj_file_destroy(file);
    return true;
}

static bool print_test(obj_settings_t *settings, int argc, char **argv){
    UNUSED(settings);

    if(!requested_arguments_exact(argc, 1)){
        return false;
    }

    char *filename = argv[0];
    obj_file_t *obj = NULL;

    if(!obj_load(filename, &obj)){
        printf("%s\r\n", filelib_error());
        return false;
    }

    printf("Object file %s\r\n", filename);
    printf(" |- arch: %s\r\n", obj->target_arch_name);

    for(unsigned i = 0; i < list_count(obj->section_list); i++){
        obj_section_t *section = NULL;
        list_at(obj->section_list, i, (void *)&section);

        char next_section = ((i + 1) != list_count(obj->section_list))  ? '|' : ' ';

        if(next_section == ' '){
            printf(" '- Section %s\r\n", section->section_name);
        }
        else{
            printf(" |- Section %s\r\n", section->section_name);
        }

        printf(" %c   |- Exported:\r\n", next_section);

        if(list_count(section->exported_symbol_list) == 0){
            printf(" %c   |   '- <empty>\r\n", next_section);
        }
        else{
            for(unsigned j = 0; j < list_count(section->exported_symbol_list); j++){
                obj_symbol_t *symbol = NULL;
                list_at(section->exported_symbol_list, j, (void *)&symbol);

                char *value = platformlib_write_isa_address(symbol->value);
                char next_symbol = ((j + 1) != list_count(section->exported_symbol_list))  ? '|' : '\'';

                printf(" %c   |   %c- %s %s\r\n", next_section, next_symbol, symbol->name, value);

                dynmem_free(value);
            }
        }

        printf(" %c   |- Imported:\r\n", next_section);

        if(list_count(section->imported_symbol_list) == 0){
            printf(" %c   |   '- <empty>\r\n", next_section);
        }
        else{
            for(unsigned j = 0; j < list_count(section->imported_symbol_list); j++){
                obj_symbol_t *symbol = NULL;
                list_at(section->imported_symbol_list, j, (void *)&symbol);

                char *value = platformlib_write_isa_address(symbol->value);
                char next_symbol = ((j + 1) != list_count(section->imported_symbol_list))  ? '|' : '\'';

                printf(" %c   |   %c- %s %s\r\n", next_section, next_symbol, symbol->name, value);

                dynmem_free(value);
            }
        }

        printf(" %c   '- Data:\r\n", next_section);

        if(list_count(section->data_symbol_list) == 0){
            printf(" %c       '- <empty>\r\n", next_section);
        }
        else{
            for(unsigned j = 0; j < list_count(section->data_symbol_list); j++){
                obj_data_t *symbol = NULL;
                list_at(section->data_symbol_list, j, (void *)&symbol);

                char *address = platformlib_write_isa_address(symbol->address);
                char *value = NULL;
                char next_data = ((j + 1) != list_count(section->data_symbol_list))  ? '|' : '\'';

                if(symbol->blob == true){
                    value = platformlib_write_isa_memory_element(symbol->payload.blob_value);
                    printf(" %c       %c- blob %s %s\r\n", next_section, next_data, address, value);
                }
                else{
                    value = platformlib_write_isa_instruction_word(symbol->payload.data_value);
                    char *relocation = symbol->relocation ? "1" : "0";
                    char *special = symbol->special ? "1" : "0";
                    char *special_value = platformlib_write_isa_address(symbol->special_value);

                    printf(" %c       %c- inst %s %s (%s) relocation:%s special:%s\r\n", next_section, next_data, address, value, special_value, relocation, special);

                    dynmem_free(special_value);
                }

                dynmem_free(address);
                dynmem_free(value);
            }
        }

    }

    obj_file_destroy(obj);
    return true;
}

static bool load_save_test(obj_settings_t *settings, int argc, char **argv){
    UNUSED(settings);

    if(!requested_arguments_exact(argc, 2)){
        return false;
    }

    char *filename_input = argv[0];
    char *filename_output = argv[1];

    obj_file_t *obj = NULL;

    if(!obj_load(filename_input, &obj)){
        printf("%s\r\n", filelib_error());
        return false;
    }

    if(!obj_write(obj, filename_output)){
        obj_file_destroy(obj);
        printf("%s\r\n", filelib_error());
        return false;
    }

    obj_file_destroy(obj);

    return true;
}

void obj_test_args_init(options_t *args, obj_settings_t *settings){
    options_append_section(args, "OBJ Tests", NULL);
    options_append_flag_2(args, "obj-generate", "Generate one small object file.");
    options_append_flag_2(args, "obj-print", "Print content of object file.");
    options_append_flag_2(args, "obj-load-save", "Load object file and save it again as another file.");

    settings->generate = false;
    settings->print = false;
    settings->load_save = false;
}

void obj_test_args_parse(options_t *args, obj_settings_t *settings){
    if(options_is_flag_set(args, "obj-generate")){
        settings->generate = true;
    }
    if(options_is_flag_set(args, "obj-print")){
        settings->print = true;
    }
    if(options_is_flag_set(args, "obj-load-save")){
        settings->load_save = true;
    }
}

bool obj_test_should_run(obj_settings_t *settings){
    return (settings->generate || settings->load_save || settings->print);
}

bool obj_test_run(obj_settings_t *settings, int argc, char **argv){
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
