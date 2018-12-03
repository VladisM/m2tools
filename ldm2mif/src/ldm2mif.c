#include "ldm2mif.h"

#include "error.h"

#include <mif.h>
#include <ldm.h>
#include <stdlib.h>
#include <stdio.h>


int ldm2mif(ldm_buffer_item_t* lb, mif_buffer_item_t** mb, unsigned int size_pow){

    mif_buffer_item_t* mif_current = NULL;
    mif_buffer_item_t* mif_buf_top = NULL;
    unsigned int size = 1;

    //calculate count of items in mif
    for(unsigned int i = 0; i < size_pow; i++) size *= 2;


    for(unsigned int i = 0; i < size; i++){
        mif_current = (mif_buffer_item_t *)malloc(sizeof(mif_buffer_item_t));

        if(lb != NULL){
            mif_current->value = lb->value;
            mif_current->address = lb->address;
            lb = lb->next;
        }
        else{
            mif_current->value = 0x00000000;
            mif_current->address = i;
        }

        if(mif_buf_top == NULL){
            mif_buf_top = mif_current;
            *mb = mif_buf_top;
        }
        else{
            mif_buf_top->next = mif_current;
            mif_buf_top = mif_buf_top->next;
        }
    }

    mif_buf_top->next = NULL;

    return 0;
}
