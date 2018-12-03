#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "ldm.h"

/*
 * ---------------------------------------------------------------------
 * Macro definitions
 *
 */

#define SET_ERROR(n) if(ldmlib_errno == 0) ldmlib_errno = n

/*
 *
 * End of macro definitions
 * ---------------------------------------------------------------------
 * Static functions declarations
 *
 */

static ldm_buffer_item_t* line_read(FILE* fp);
static int decode_line(ldm_buffer_item_t* i, char* line);
static FILE* ldm_open(char* filename, const char* mode);
static void ldm_close(FILE* filename);

/*
 *
 * End of statit functions declarations
 * ---------------------------------------------------------------------
 * Static variables
 *
 */

static tLdmError ldmlib_errno;

/*
 *
 * End of static variables
 * ---------------------------------------------------------------------
 * Static functions definitions
 *
 */

static ldm_buffer_item_t* line_read(FILE* fp){
    /*
     * return NULL on failure
     * othervise pointer to new buffer item is returned
     */

    char line_buff[32];
    ldm_buffer_item_t* i = (ldm_buffer_item_t*)malloc(sizeof(ldm_buffer_item_t));

    if(i == NULL){
        SET_ERROR(LDMERR_MALLOC_FAILED);
        return NULL;
    }

    if(fgets(line_buff, sizeof line_buff, fp) == NULL){
        if(feof(fp)){
            //toto je důvod proč to hodí error - poslední řádek je prázdný řádek a fgets vrátí NULL
            SET_ERROR(LDMERR_READING_EMPTY_LINE);
        }
        else{
            SET_ERROR(LDMERR_LINE_READ_FAILED);
        }
        free(i);
        return NULL;
    }

    if(!decode_line(i, line_buff)){
        free(i);
        return NULL;
    }

    return i;
}

static int decode_line(ldm_buffer_item_t* i, char* line){
    /*
     * return 0 on failure
     * othervise 1
     *
     * will set libldm_errno
     */

    char buff[32];
    unsigned int adr, val = 0;

    int ret = sscanf(line, "%x:%x:%s", &adr, &val, buff);

    if(ret != 3){
        SET_ERROR(LDMERR_BAD_LDM_FORMAT);
        return 0;
    }

    i->address = adr;
    i->value = val;

    if(strcmp(buff, "True") == 0){
        i->relocation = 1;
    }
    else if(strcmp(buff, "False") == 0){
        i->relocation = 0;
    }
    else{
        SET_ERROR(LDMERR_BAD_LDM_FORMAT);
        return 0;
    }

    return 1;
}

static FILE* ldm_open(char* filename, const char* mode){

    FILE* fp = fopen(filename, mode);

    if(!fp){
        SET_ERROR(LDMERR_FILE_OPEN_ERROR);
        return NULL;
    }

    return fp;
}

static void ldm_close(FILE* filename){
    fclose(filename);
}

/*
 *
 * End of static functions
 * ---------------------------------------------------------------------
 * Exported functions definitions
 *
 */

int ldm_load(char* filename, ldm_buffer_item_t** b){
    //read conent of file fp into buffer b

    FILE* fp;
    ldm_buffer_item_t* item_ptr;
    ldm_buffer_item_t* last_item = *b;

    fp = ldm_open(filename, "r");

    if(fp == NULL){
        SET_ERROR(LDMERR_FILE_NOT_OPENNED);
        return -1;
    }

    unsigned int last_address = 0;
    int first_run = 1;

    while(!feof(fp)){

        item_ptr = line_read(fp);
        if(item_ptr == NULL){
            if(get_ldmlib_errno() == LDMERR_READING_EMPTY_LINE){
                clear_ldmlib_errno();
                continue;
            }
            else{
                ldm_close(fp);
                return -1;
            }
        }

        if(first_run == 0){
            last_address++;
        }
        else{
            first_run = 0;
        }

        if(item_ptr->address != last_address){
            //add missing null items
            do{
                ldm_buffer_item_t* empty_item = (ldm_buffer_item_t*)malloc(sizeof(ldm_buffer_item_t));

                empty_item->address = last_address;
                empty_item->relocation = 0;
                empty_item->value = 0;

                if(last_item != NULL){
                    last_item->next = empty_item;
                    last_item = last_item->next;
                }
                else{
                    last_item = empty_item;
                    *b = last_item;
                }
            }while(item_ptr->address != ++last_address);

        }

        //add item at the top of b buffer
        if(last_item != NULL){
            last_item->next = item_ptr;
            last_item = last_item->next;
        }
        else{
            last_item = item_ptr;
            *b = last_item;
        }
    }

    ldm_close(fp);
    return 0;
}

tLdmError get_ldmlib_errno(void){
    return ldmlib_errno;
}

void clear_ldmlib_errno(void){
    ldmlib_errno = 0;
}

int free_ldm_buffer(ldm_buffer_item_t* b){
    if(b != NULL){
        ldm_buffer_item_t *actual = b;
        ldm_buffer_item_t *last = NULL;

        while(actual->next != NULL){
            last = actual;
            actual = actual->next;
            free(last);
        }
        free(actual);
        return 0;
    }
    else{
        SET_ERROR(LDMERR_CLEAR_EMPTY_BUFFER);
        return -1;
    }
}

int ldm_write(char* filename, ldm_buffer_item_t* b){

    FILE* fp;
    char rel_buff[8];

    fp = ldm_open(filename, "w");

    //check file pointer
    if(fp == NULL){
        SET_ERROR(LDMERR_FILE_NOT_OPENNED);
        return -1;
    }

    //loop over all items in buffer
    while(b != NULL){

        if(b->value == 0){
            //skip items witch value is 0
            b = b->next;
            continue;
        }

        //prepare relocation string
        if(b->relocation == 1){
            strcpy(rel_buff, "True");
        }
        else{
            strcpy(rel_buff, "False");
        }

        //write data into file
        int ret = fprintf(fp, "0x%x:0x%x:%s\n", b->address, b->value, rel_buff);

        //check return code
        if(ret < 0){
            SET_ERROR(LDMERR_WRITING_FAILED);
            ldm_close(fp);
            return -1;
        }

        //iteration over all items in buffer
        b = b->next;
    }

    ldm_close(fp);
    return 0;
}

/*
 *
 * End of exported functions definitions
 * ---------------------------------------------------------------------
 */
