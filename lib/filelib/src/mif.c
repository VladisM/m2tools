#include "_filelib.h"

bool mif_write(mif_file_t *f, char *filename){
    return _write_file(filename, (void **)f, &check_structure_mif, &writing_loop_mif);
}

void mif_file_new(mif_file_t **f){
    CHECK_NULL_ARGUMENT(f);
    CHECK_NOT_NULL_ARGUMENT(*f);

    *f = (mif_file_t *)dynmem_malloc(sizeof(mif_file_t));

    (*f)->content = NULL;
    (*f)->settings.data_radix = RADIX_UNK;
    (*f)->settings.data_width = 0;
    (*f)->settings.address_radix = RADIX_UNK;
    (*f)->settings.depth = 0;

    list_init(&(*f)->content, sizeof(mif_item_t *));
}

void mif_file_destroy(mif_file_t *f){
    CHECK_NULL_ARGUMENT(f);

    if(f->content != NULL){
        while(list_count(f->content) > 0){
            ldm_item_t *tmp = NULL;
            list_windraw(f->content, (void *)&tmp);
            dynmem_free(tmp);
        }
    }
}

void mif_config_data(mif_file_t *f, unsigned data_width){
    CHECK_NULL_ARGUMENT(f);
    f->settings.data_width = data_width;
}

void mif_config_address(mif_file_t *f, unsigned address_depth){
    CHECK_NULL_ARGUMENT(f);
    f->settings.depth = address_depth;
}

bool mif_config_radixes(mif_file_t *f, mif_radix_t data_radix, mif_radix_t address_radix){
    CHECK_NULL_ARGUMENT(f);

    if(data_radix == RADIX_UNK){
        FILELIB_ERROR_WRITE("Data radix cannot be undefined or unknown!");
        return false;
    }

    if(address_radix == RADIX_UNK){
        FILELIB_ERROR_WRITE("Address radix cannot be undefined or unknown!");
        return false;
    }

    f->settings.data_radix = data_radix;
    f->settings.address_radix = address_radix;

    return true;
}

void mif_item_new(mif_item_t **item, isa_address_t address, isa_memory_element_t data){
    CHECK_NULL_ARGUMENT(item);
    CHECK_NOT_NULL_ARGUMENT(*item);

    *item = (mif_item_t *)dynmem_malloc(sizeof(mif_item_t));

    (*item)->address = address;
    (*item)->data = data;
}

void mif_item_destroy(mif_item_t *item){
    CHECK_NULL_ARGUMENT(item);

    dynmem_free(item);
}

void mif_item_into_file(mif_file_t *file, mif_item_t *item){
    CHECK_NULL_ARGUMENT(file);
    CHECK_NULL_ARGUMENT(item);

    list_append(file->content, (void *)&item);
}
