/**
 * @file ln_section_list.c
 *
 * @brief Portable linker from m2tools.
 *
 * @author Bc. Vladislav Mlejnecký <v.mlejnecky@seznam.cz>
 * @date 07.02.2020
 *
 * @note This file is part of m2tools project.
 *
 * Linker is splitted into multiple files:
 *  - ldparser.c
 *  - ldparser.h
 *  - linker.c
 *  - linker_util.c
 *  - linker_util.h
 *  - ln_section_list.c
 *  - ln_section_lish.h
 *  - ln_symbol_list.c
 *  - ln_symbol_list.h
 */

#include "ln_section_list.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include <isa.h>
#include <obj.h>
#include <ldm.h>
#include <sl.h>

#include "linker_util.h"

#define SET_ERROR(n) if(section_list_errno == SECTION_OK) section_list_errno = n

/**
 * @brief Variable where pointer to list is stored.
 */
section_list_item_t *first_section_item = NULL;

/**
 * @brief Variable for error code.
 */
static ln_section_list_errno_t section_list_errno = SECTION_OK;

/**
 * @brief Create deep copy of section.
 *
 * That mean - every object stored in previous structure will be recreated again.
 */
static bool make_deep_copy_of_section(section_t *section_in, section_t **section_out);

/**
 * @brief Append section into our list in first_section_item.
 *
 * @return true if append OK; false if problem occurs (if same section exist)
 */
static bool append_into_list(section_t *section);

/**
 * @brief Compare two sections if are the same.
 *
 * @return true if same; false if not
 */
static bool is_same_section(section_t *A, section_t *B);

/**
 * @brief Create new structure for holding section.
 */
static bool create_new_section_list_item(section_list_item_t **item_ptr);

/**
 * @brief Merge section B into section A.
 */
static bool merge_sections(section_t *A, section_t *B);

/**
 * @brief Clean up memory ocupied by pointed section list item. Also clean-up
 * associated obj_section.
 */
static void free_section_list_item(section_list_item_t *s);

/**
 * @brief Remove specified section from list of sections.
 *
 * List of section si defined by first_section_item variable.
 */
static bool remove_section(section_list_item_t *s);

bool append_into_section_list_sl(static_library_t *sl){
    if(sl == NULL){
        SET_ERROR(SECTION_WRONG_ARG);
        return false;
    }

    for(obj_file_t *head = sl->first_obj; head != NULL; head = head->next){
        if(!append_into_section_list_obj(head)){
            return false;
        }
    }

    return true;
}

bool append_into_section_list_obj(obj_file_t *obj){
    if(obj == NULL){
        SET_ERROR(SECTION_WRONG_ARG);
        return false;
    }

    for(section_t *head = obj->first_section; head != NULL; head = head->next){
        section_t *tmp = NULL;

        if(!make_deep_copy_of_section(head, &tmp)) return false;
        if(!append_into_list(tmp)) return false;
    }

    return true;
}

static bool make_deep_copy_of_section(section_t *section_in, section_t **section_out){
    section_t *new_sec = NULL;

    if(!new_section(section_in->section_name, &new_sec)){
        SET_ERROR(SECTION_OBJLIB_ERROR);
        free_obj_section(new_sec);
        return false;
    }

    for(spec_symbol_t *head = section_in->spec_symbol_first; head != NULL; head = head->next){
        spec_symbol_t *new_symbol = NULL;

        if(!new_spec_symbol(head->name, head->value, head->type, &new_symbol)){
            SET_ERROR(SECTION_OBJLIB_ERROR);
            free_obj_section(new_sec);
            return false;
        }

        if(!append_spec_symbol_to_section(new_sec, new_symbol)){
            SET_ERROR(SECTION_OBJLIB_ERROR);
            free_obj_spec_symbol(new_symbol);
            free_obj_section(new_sec);
            return false;
        }
    }

    for(data_symbol_t *head = section_in->data_first; head != NULL; head = head->next){
        data_symbol_t *new_symbol = NULL;

        if(head->type == DATA_IS_BLOB){
            datablob_t *blob = NULL;

            if(new_blob(head->payload.blob->lenght, &blob)){
                SET_ERROR(SECTION_OBJLIB_ERROR);
                free_obj_section(new_sec);
                return false;
            }

            for(unsigned int i = 0; i < blob->lenght; i++){
                blob->payload[i] = head->payload.blob->payload[i];
            }

            if(!new_data_symbol(head->address, DATA_IS_BLOB, (void *)blob, &new_symbol)){
                SET_ERROR(SECTION_OBJLIB_ERROR);
                free_obj_blob_payload(blob);
                free_obj_section(new_sec);
                return false;
            }
        }
        else if(head->type == DATA_IS_INST){
            tInstruction *i = new_instru();

            if(i == NULL){
                SET_ERROR(SECTION_ISALIB_ERROR);
                free_obj_section(new_sec);
                return false;
            }

            i->word = head->payload.inst->word;

            if(!new_data_symbol(head->address, DATA_IS_INST, (void *)i, &new_symbol)){
                SET_ERROR(SECTION_OBJLIB_ERROR);
                free_istruction_struct(i);
                free_obj_section(new_sec);
                return false;
            }
        }
        else{
            exit(EXIT_FAILURE);
        }

        new_symbol->address = head->address;
        new_symbol->relocation = head->relocation;
        new_symbol->special = head->special;

        if(!append_data_symbol_to_section(new_sec, new_symbol)){
            SET_ERROR(SECTION_OBJLIB_ERROR);
            free_obj_data_symbol(new_symbol);
            free_obj_section(new_sec);
            return false;
        }
    }

    *section_out = new_sec;

    return true;
}

static bool append_into_list(section_t *section){
    section_list_item_t *head = NULL;
    section_list_item_t *new_holder = NULL;

    if(first_section_item == NULL){
        if(!create_new_section_list_item(&new_holder)) return false;
        new_holder->section = section;

        first_section_item = new_holder;
    }
    else{
        for(head = first_section_item; head != NULL; head = head->next){
            if(is_same_section(head->section, section)){
                if(!merge_sections(head->section, section)) return false;
                break;
            }

            if(head->next == NULL){
                if(!create_new_section_list_item(&new_holder)) return false;
                new_holder->section = section;

                head->next = new_holder;
                new_holder->prev = head;
                break;
            }
        }
    }

    return true;
}

static bool create_new_section_list_item(section_list_item_t **item_ptr){
    section_list_item_t *tmp = NULL;

    tmp = (section_list_item_t *)malloc(sizeof(section_list_item_t));

    check_malloc((void *)tmp);

    tmp->section = NULL;
    tmp->assinged_mem = NULL;
    tmp->next = NULL;
    tmp->prev = NULL;
    tmp->begin_addr = 0;
    tmp->used = false;

    *item_ptr = tmp;

    return true;
}

static bool is_same_section(section_t *A, section_t *B){
    if(strcmp(A->section_name, B->section_name) == 0){
        return true;
    }
    return false;
}

static bool merge_sections(section_t *A, section_t *B){

    //get informations about section A
    isa_address_t addr_offset = 0;
    unsigned int import_label_counter = 0;

    addr_offset = get_section_size(A);

    for(spec_symbol_t *head = A->spec_symbol_first; head != NULL; head = head->next){
        if(head->type == SYMBOL_IMPORT){
            import_label_counter++;
        }
    }

    for(spec_symbol_t *head = B->spec_symbol_first; head != NULL; head = head->next){
        spec_symbol_t *tmp = NULL;
        isa_address_t offset = 0;

        if(head->type == SYMBOL_EXPORT){
            offset = addr_offset;
        }
        else if(head->type == SYMBOL_IMPORT){
            offset = import_label_counter;
        }
        else{
            fprintf(stderr, "Special symbol have to by export type or import type! Probably broken assembler?\n");
            exit(EXIT_FAILURE);
        }

        //add it into section A
        if(!new_spec_symbol(head->name, head->value + offset, head->type, &tmp)){
            SET_ERROR(SECTION_OBJLIB_ERROR);
            return false;
        }

        if(!append_spec_symbol_to_section(A, tmp)){
            free_obj_spec_symbol(tmp);
            SET_ERROR(SECTION_OBJLIB_ERROR);
            return false;
        }
    }

    for(data_symbol_t *head = B->data_first; head != NULL; head = head->next){
        data_symbol_t *tmp = NULL;
        void *payload_ptr = NULL;

        if(head->type == DATA_IS_BLOB){
            datablob_t *tmp_payload = NULL;

            if(!new_blob(head->payload.blob->lenght, &tmp_payload)){
                SET_ERROR(SECTION_OBJLIB_ERROR);
                return false;
            }

            for(unsigned int i = 0; i < head->payload.blob->lenght; i++){
                tmp_payload->payload[i] = head->payload.blob->payload[i];
            }

            payload_ptr = (void *)tmp_payload;
        }

        if(head->type == DATA_IS_INST){
            tInstruction *tmp_payload = new_instru();

            if(tmp_payload == NULL){
                SET_ERROR(SECTION_ISALIB_ERROR);
                return false;
            }

            tmp_payload->word = head->payload.inst->word;

            if(head->special == true){
                if(!relocate_instruction(tmp_payload, import_label_counter)){
                    SET_ERROR(SECTION_ISALIB_ERROR);
                    return false;
                }
            }

            if(head->relocation == true){
                if(!relocate_instruction(tmp_payload, addr_offset)){
                    SET_ERROR(SECTION_ISALIB_ERROR);
                    return false;
                }
            }

            payload_ptr = (void *)tmp_payload;
        }

        if(!new_data_symbol(head->address + addr_offset, head->type, payload_ptr, &tmp)){

            if(head->type == DATA_IS_BLOB){
                free(((datablob_t *)payload_ptr)->payload);
                free(payload_ptr);
            }
            if(head->type == DATA_IS_INST){
                free_istruction_struct((tInstruction *)payload_ptr);
            }

            SET_ERROR(SECTION_OBJLIB_ERROR);
            return false;
        }

        tmp->relocation = head->relocation;
        tmp->special = head->special;

        if(!append_data_symbol_to_section(A, tmp)){
            free_obj_data_symbol(tmp);
            SET_ERROR(SECTION_OBJLIB_ERROR);
            return false;
        }
    }

    return true;
}

void clear_section_list_errno(void){
    section_list_errno = SECTION_OK;
}

ln_section_list_errno_t get_section_list_errno(void){
    return section_list_errno;
}

void clean_up_section_list(void){
    section_list_item_t *head = NULL;
    section_list_item_t *tmp = NULL;

    head = first_section_item;
    while(head != NULL){
        tmp = head;
        head = head->next;

        free_section_list_item(tmp);
    }
}


bool strip_unused_sections(void){
    if(first_section_item == NULL){
        fprintf(stderr, "There is no section in the list.\n");
        SET_ERROR(SECTION_LIST_EMPTY);
        return false;
    }

    section_list_item_t *head = first_section_item;
    section_list_item_t *tmp = NULL;

    while(head != NULL){
        tmp = head;
        head = head->next;

        if(tmp->used == false){
            if(!remove_section(tmp)){
                return false;
            }
        }
    }

    fprintf(stderr, "Strip down unused sections is not implemented yet!\n");
    return false;
}

static void free_section_list_item(section_list_item_t *s){
    if(s == NULL){
        return;
    }

    free_obj_section(s->section);
    free(s);
}

static bool remove_section(section_list_item_t *s){
    if(first_section_item == NULL || s == NULL){
        SET_ERROR(SECTION_WRONG_ARG);
        fprintf(stderr, "Should remove section but there is no section in list or NULL was given.\n");
        return false;
    }

    if(first_section_item == s){
        first_section_item = s->next;
    }

    if(s->next != NULL){
        s->next->prev = s->prev;
    }

    if(s->prev != NULL){
        s->prev->next = s->next;
    }

    free_section_list_item(s);
    return true;
}


#ifndef NDEBUG
void print_section_list(void){
    printf("Section cache:\n");

    if(first_section_item == NULL){
        printf("'- (null)\n");
        return;
    }

    for(section_list_item_t *head = first_section_item; head != NULL; head = head->next){
        if(head->next != NULL){
            printf("|- Name: %s\n", head->section->section_name);

            if(head->assinged_mem == NULL)
                printf("|  |- mem: (null)\n");
            else
                printf("|  |- mem: %s\n", head->assinged_mem->mem_name);

            printf("|  |- begin address: "PRIisa_addr"\n", head->begin_addr);
            printf("|  |- size: "PRIisa_addr"\n", get_section_size(head->section));
            printf("|  '- used: %s\n", head->used ? "true" : "false");
        }
        else{
            printf("'- Name: %s\n", head->section->section_name);

            if(head->assinged_mem == NULL)
                printf("   |- mem: (null)\n");
            else
                printf("   |- mem: %s\n", head->assinged_mem->mem_name);

            printf("   |- begin address: "PRIisa_addr"\n", head->begin_addr);
            printf("   |- size: "PRIisa_addr"\n", get_section_size(head->section));
            printf("   '- used: %s\n", head->used ? "true" : "false");
        }
    }
}

void print_section_status(void){
    printf("Used sections summary:\n");

    if(first_section_item == NULL){
        printf("'- (null)\n");
        return;
    }

    for(section_list_item_t *head = first_section_item; head != NULL; head = head->next){
        char c = '\0';

        if(head->next != NULL) c = '|';
        else c = '\'';

        printf("%c- Section '%s' - %s\n", c, head->section->section_name, head->used ? "USED" : "UNUSED");
    }
}
#endif
