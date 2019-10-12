#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <ldm.h>

#define FAIL(x) fprintf(stderr, "Err: '%s' ldm_errno: %d\n", x, get_ldmlib_errno()); exit(EXIT_FAILURE)

int main(int argc, char **argv){

    ldm_file_t *lfile = NULL;
    ldm_mem_t *mem_1 = NULL;
    ldm_mem_t *mem_2 = NULL;

    ldm_item_t *item_1m1 = NULL;
    ldm_item_t *item_2m1 = NULL;
    ldm_item_t *item_3m1 = NULL;
    ldm_item_t *item_1m2 = NULL;
    ldm_item_t *item_2m2 = NULL;
    ldm_item_t *item_3m2 = NULL;

    if(new_ldm_file(&lfile, "test.ldm", 0x100) != true){
        FAIL("failed create file!");
    }

    if(new_mem(&mem_1, "mem_1", 0x000, 256) != true){
        FAIL("failed create mem_1");
    }

    if(new_mem(&mem_2, "mem_2", 0x100, 256) != true){
        FAIL("failed create mem_2!");
    }

    if(append_mem_into_file(mem_1, lfile) != true){
        FAIL("failed to add mem_1 into ldm file!");
    }

    if(append_mem_into_file(mem_2, lfile) != true){
        FAIL("failed to add mem_2 into ldm file!");
    }

    if(new_item(&item_1m1, 0x001, 1) != true){
        FAIL("failed to create item_1m1!");
    }

    if(new_item(&item_2m1, 0x002, 2) != true){
        FAIL("failed to create item_2m1!");
    }

    if(new_item(&item_3m1, 0x003, 3) != true){
        FAIL("failed to create item_3m1!");
    }

    if(new_item(&item_1m2, 0x101, 1) != true){
        FAIL("failed to create item_1m2!");
    }

    if(new_item(&item_2m2, 0x102, 2) != true){
        FAIL("failed to create item_2m2!");
    }

    if(new_item(&item_3m2, 0x103, 3) != true){
        FAIL("failed to create item_3m2!");
    }

    if(append_item_into_mem(item_1m1, mem_1) != true){
        FAIL("failed to append item_1m1!");
    }

    if(append_item_into_mem(item_2m1, mem_1) != true){
        FAIL("failed to append item_2m1!");
    }

    if(append_item_into_mem(item_3m1, mem_1) != true){
        FAIL("failed to append item_3m1!");
    }

    if(append_item_into_mem(item_1m2, mem_2) != true){
        FAIL("failed to append item_1m2!");
    }

    if(append_item_into_mem(item_2m2, mem_2) != true){
        FAIL("failed to append item_2m2!");
    }

    if(append_item_into_mem(item_3m2, mem_2) != true){
        FAIL("failed to append item_3m2!");
    }

    if(ldm_write("test.ldm", lfile) != true){
        FAIL("failed to writeout file!");
    }

    free_ldm_file_struct(lfile);

    printf("ldmtest - OK\n");

    return 0;
}
