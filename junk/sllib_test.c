#include <sl.h>
#include <obj.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(){
    obj_file_t *o1 = NULL;
    obj_file_t *o2 = NULL;
    static_library_t *sl1= NULL;
    static_library_t *sl2= NULL;

    obj_load("test_1.o", &o1);
    obj_load("test_2.o", &o2);

    new_sl("test_1.sl", &sl1);
    append_objfile_to_sl(o1, sl1);
    append_objfile_to_sl(o2, sl1);
    sl_write("test_1.sl", sl1);

    sl_load("test_1.sl", &sl2);
    strcpy(sl2->library_name, "test_2.sl");
    sl_write("test_2.sl", sl2);

    //~ free_object_file(o1);
    //~ free_object_file(o2);
    free_sl(sl1);
    free_sl(sl2);

    return 1;
}
