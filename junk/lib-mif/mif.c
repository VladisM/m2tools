#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "mif.h"

/*
 * ---------------------------------------------------------------------
 * Macro definitions
 *
 */

#define SET_ERROR(n) if(miflib_errno == 0) miflib_errno = n

typedef enum{
    RADIX_UNDEFINED = 0,
    RADIX_HEX,
    RADIX_BIN,
    RADIX_OCT,
    RADIX_UNK
} tRadix;

/*
 *
 * End of macro definitions
 * ---------------------------------------------------------------------
 * Private datatypes definitions
 *
 */

typedef struct mif_info_s{
    tRadix data_radix;
    int data_width;
    tRadix address_radix;
    int depth;
}mif_info_t;

/*
 *
 * End of private datatypes definitions
 * ---------------------------------------------------------------------
 * Static functions declarations
 *
 */

static int decode_data_line(mif_buffer_item_t* i, char* line);
static FILE* mif_open(char * filename, const char * mode);
static void mif_close(FILE* fp);
static mif_buffer_item_t* data_line_read(FILE* fp);
static int decode_settings_line(mif_info_t* info, char* line);
static mif_info_t* read_head(FILE* fp);

/*
 *
 * End of statit functions declarations
 * ---------------------------------------------------------------------
 * Static variables
 *
 */

static int miflib_errno;

/*
 *
 * End of static variables
 * ---------------------------------------------------------------------
 * Static functions definitions
 *
 */

static FILE* mif_open(char* filename, const char* mode){
    FILE* fp = fopen(filename, mode);

    if(fp == NULL){
        SET_ERROR(MIFERR_FILE_OPEN_ERROR);
        return NULL;
    }

    return fp;
}

static void mif_close(FILE* fp){
    fclose(fp);
}

static int decode_data_line(mif_buffer_item_t* i, char* line){
    //data radix have to be HEX
    //address radix too
    //there have to be no memory shortuct like this shit: [0..8] = 0xFFFFFFFF
    //data width have to be 32
    //address width can be up to 32
    //line have to be data line ... after "CONTENT BEGIN" line in mif

    uint32_t adr,val;

    int ret = sscanf(line, "%x : %x;", &adr, &val);

    if(ret != 2){
        SET_ERROR(MIFERR_BAD_MIF_FORMAT);
        return -1;
    }

    i->address = adr;
    i->value = val;

    return 0;
}

static mif_buffer_item_t* data_line_read(FILE* fp){
    /*
     * return NULL on failure
     * othervise pointer to new buffer item is returned
     */

    char line_buff[32];
    mif_buffer_item_t* i = (mif_buffer_item_t*)malloc(sizeof(mif_buffer_item_t));

    if(i == NULL){
        SET_ERROR(MIFERR_MALLOC_FAILED);
        return NULL;
    }

    if(fgets(line_buff, sizeof line_buff, fp) == NULL){
        if(feof(fp)){
            SET_ERROR(MIFERR_READING_EMPTY_LINE);
        }
        else{
            SET_ERROR(MIFERR_LINE_READ_FAILED);
        }
        free(i);
        return NULL;
    }

    if(strncmp("END", line_buff, 3) == 0){
        free(i);
        SET_ERROR(MIFERR_END_OF_FILE);
        return NULL;
    }

    if(decode_data_line(i, line_buff)){
        free(i);
        return NULL;
    }

    return i;
}

static int decode_settings_line(mif_info_t* info, char* line){
    //there have to be spaces around '='
    //there have to be only one ';' at the end of line
    //before ';' cant be any ' '

    char param[32];
    char arg[16];

    if(sscanf(line, "%s = %s", param, arg) != 2){
        SET_ERROR(MIFERR_BAD_MIF_FORMAT);
        return -1;
    }

    for(int i = 0;arg[i] != '\0'; i++){
        if(arg[i] == ';'){
            arg[i] = '\0';
            break;
        }
    }

    if(strcmp(param, "DEPTH") == 0){
        info->depth = atoi(arg);
    }
    else if(strcmp(param, "WIDTH") == 0){
        info->data_width = atoi(arg);
    }
    else if(strcmp(param, "ADDRESS_RADIX") == 0){
        if(strcmp(arg, "HEX") == 0){
            info->address_radix = RADIX_HEX;
        }
        else if(strcmp(arg, "BIN") == 0){
            info->address_radix = RADIX_BIN;
        }
        else if(strcmp(arg, "OCT") == 0){
            info->address_radix = RADIX_OCT;
        }
        else{
            info->address_radix = RADIX_UNK;
            SET_ERROR(MIFERR_BAD_MIF_FORMAT);
            return -1;
        }
    }
    else if(strcmp(param, "DATA_RADIX") == 0){
        if(strcmp(arg, "HEX") == 0){
            info->data_radix = RADIX_HEX;
        }
        else if(strcmp(arg, "BIN") == 0){
            info->data_radix = RADIX_BIN;
        }
        else if(strcmp(arg, "OCT") == 0){
            info->data_radix = RADIX_OCT;
        }
        else{
            info->data_radix = RADIX_UNK;
            SET_ERROR(MIFERR_BAD_MIF_FORMAT);
            return -1;
        }
    }
    else{
        SET_ERROR(MIFERR_BAD_MIF_FORMAT);
        return -1;
    }

    return 0;
}

static mif_info_t* read_head(FILE* fp){
    //comments are not ignored!
    //there should not be free spaces before command on the line

    char line_buff[64];

    mif_info_t* info = (mif_info_t*) malloc(sizeof(mif_info_t));

    if(info == NULL){
        SET_ERROR(MIFERR_MALLOC_FAILED);
        return NULL;
    }

    info->data_radix = RADIX_UNK;
    info->address_radix = RADIX_UNK;
    info->depth = 0;
    info->data_width = 0;

    while(1){
        fgets(line_buff, sizeof line_buff, fp);

        if(strcmp(line_buff, "\n") == 0) continue; //skip empty lines
        if(strcmp(line_buff, "CONTENT\n") == 0) continue; //sequence "CONTENT\nBEGIN" is also valid!
        if(strcmp(line_buff, "BEGIN\n") == 0) break;
        if(strcmp(line_buff, "CONTENT BEGIN\n") == 0) break; //break when 'CONTENT BEGIN' found

        decode_settings_line(info, line_buff);
    }

    return info;
}

/*
 *
 * End of static functions
 * ---------------------------------------------------------------------
 * Exported functions definitions
 *
 */

tMifError get_miflib_errno(void){
    return miflib_errno;
}

void clear_miflib_errno(void){
    miflib_errno = 0;
}

int mif_load(char * filename, mif_buffer_item_t** mif_buffer){

    mif_info_t* mif_info;
    FILE* fp;

    fp = mif_open(filename, "r");

    mif_info = read_head(fp);

    if(mif_info->data_radix != RADIX_HEX){
        SET_ERROR(MIFERR_BAD_MIF_FORMAT);
        mif_close(fp);
        return -1;
    }

    if(mif_info->address_radix != RADIX_HEX){
        SET_ERROR(MIFERR_BAD_MIF_FORMAT);
        mif_close(fp);
        return -1;
    }

    if(mif_info->data_width != 32){
        SET_ERROR(MIFERR_BAD_MIF_FORMAT);
        mif_close(fp);
        return -1;
    }

    while(1){
        mif_buffer_item_t* tmp_mif_item;

        tmp_mif_item = data_line_read(fp);

        if(tmp_mif_item != NULL){
            //add item at the top of b buffer
            mif_buffer_item_t* last_item;

            if(*mif_buffer != NULL){
                for(last_item = *mif_buffer; last_item->next != NULL; last_item = last_item->next);
                last_item->next = tmp_mif_item;
            }
            else{
                *mif_buffer = tmp_mif_item;
            }
        }
        else{
            if(get_miflib_errno() == MIFERR_END_OF_FILE){
                break; //reached "END" in mif
            }
            else{
                mif_close(fp);
                return -1;
            }
        }
    }

    free(mif_info);
    mif_close(fp);
    return 0;
}

int mif_write(char * filename, mif_buffer_item_t* mif_buffer){
    FILE* fp;

    fp = mif_open(filename, "w");

    if(fp == NULL){
        SET_ERROR(MIFERR_FILE_OPEN_ERROR);
        return -1;
    }

    mif_buffer_item_t *b = mif_buffer;
    int i = 0;
    while(b != NULL){
        i++;
        b = b->next;
    }

    //print mif head
    fprintf(fp, "DEPTH = %d;\n", i);
    fprintf(fp, "WIDTH = 32;\n");
    fprintf(fp, "ADDRESS_RADIX = HEX;\n");
    fprintf(fp, "DATA_RADIX = HEX;\n");
    fprintf(fp, "\nCONTENT BEGIN\n");

    //print mif body with data
    for(;mif_buffer != NULL; mif_buffer = mif_buffer->next){
        fprintf(fp, "%x : %x;\n", mif_buffer->address, mif_buffer->value);
    }

    fprintf(fp, "END;\n");

    mif_close(fp);
    return 0;
}

int free_mif_buffer(mif_buffer_item_t* mif_buffer){
    if(mif_buffer != NULL){
        mif_buffer_item_t *actual = mif_buffer;
        mif_buffer_item_t *last = NULL;

        while(actual->next != NULL){
            last = actual;
            actual = actual->next;
            free(last);
        }
        free(actual);
        return 0;
    }
    else{
        SET_ERROR(MIFERR_CLEAR_EMPTY_BUFFER);
        return -1;
    }
}
