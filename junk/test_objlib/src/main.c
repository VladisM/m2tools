#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <obj.h>

int main(int argc, char **argv){

    obj_file_t * my_obj = NULL;
    obj_file_t * my_obj_2 = NULL;
    section_t * my_section_1 = NULL;
    section_t * my_section_2 = NULL;
    spec_symbol_t * my_symbol_e = NULL;
    spec_symbol_t * my_symbol_i = NULL;

    if(new_obj("test.o", &my_obj)){
        fprintf(stderr, "Failed to create obj! error_no: %d\n", get_objlib_errno());
        exit(EXIT_FAILURE);
    }

    if(new_section("section_1", &my_section_1)){
        fprintf(stderr, "Failed to create section 1! error_no %d\n", get_objlib_errno());
        exit(EXIT_FAILURE);
    }

    if(new_section("section_2", &my_section_2)){
        fprintf(stderr, "Failed to create section 2! error_no %d\n", get_objlib_errno());
        exit(EXIT_FAILURE);
    }

    if(new_spec_symbol("test_symbol", 1, SYMBOL_IMPORT, &my_symbol_i)){
        fprintf(stderr, "Failed to create special import symbol! error_no %d\n", get_objlib_errno());
        exit(EXIT_FAILURE);
    }

    if(new_spec_symbol("test_symbol", 1, SYMBOL_EXPORT, &my_symbol_e)){
        fprintf(stderr, "Failed to create special export symbol! error_no %d\n", get_objlib_errno());
        exit(EXIT_FAILURE);
    }

    if(append_spec_symbol_to_section(my_section_1, my_symbol_e)){
        fprintf(stderr, "Failed to add export symbol to section 1! error_no %d\n", get_objlib_errno());
        exit(EXIT_FAILURE);
    }

    if(append_spec_symbol_to_section(my_section_2, my_symbol_i)){
        fprintf(stderr, "Failed to add import symbol to section 2! error_no %d\n", get_objlib_errno());
        exit(EXIT_FAILURE);
    }

    if(append_section_to_obj(my_obj, my_section_1)){
        fprintf(stderr, "Failed append section 1 into object! error_no %d\n", get_objlib_errno());
        exit(EXIT_FAILURE);
    }

    if(append_section_to_obj(my_obj, my_section_2)){
        fprintf(stderr, "Failed append section 2 into object! error_no %d\n", get_objlib_errno());
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < 10; i++){
        data_symbol_t *data_symbol = NULL;

        if(new_data_symbol(i * 4, i, 0, 0, &data_symbol)){
            fprintf(stderr, "Failed to create data symbol for section 1 in iteration %d! error_no %d\n", i, get_objlib_errno());
            exit(EXIT_FAILURE);
        }

        if(append_data_symbol_to_section(my_section_1, data_symbol)){
            fprintf(stderr, "Failed append new data symbol into section 1 in iteration %d! error_no %d\n", i, get_objlib_errno());
            exit(EXIT_FAILURE);
        }

        if(new_data_symbol(100 + (i * 4), i, 0, 0, &data_symbol)){
            fprintf(stderr, "Failed to create data symbol for section 2 in iteration %d! error_no %d\n", i, get_objlib_errno());
            exit(EXIT_FAILURE);
        }

        if(append_data_symbol_to_section(my_section_2, data_symbol)){
            fprintf(stderr, "Failed append new data symbol into section 2 in iteration %d! error_no %d\n", i, get_objlib_errno());
            exit(EXIT_FAILURE);
        }

    }

    if(obj_write("test.o", my_obj)){
        fprintf(stderr, "Failed to write object file! error_no %d\n", get_objlib_errno());
        exit(EXIT_FAILURE);
    }

    if(obj_load("test.o", &my_obj_2)){
        fprintf(stderr, "Failed to load test.o! error_no %d\n", get_objlib_errno());
        exit(EXIT_FAILURE);
    }

    if(obj_write("test_2.o", my_obj_2)){
        fprintf(stderr, "Failed to write second object file! error_no %d\n", get_objlib_errno());
        exit(EXIT_FAILURE);
    }

    free_object_file(my_obj);
    free_object_file(my_obj_2);

    return 0;
}
