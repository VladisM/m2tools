#include <pass1.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <symbol_table.h>
#include <tokenizer.h>
#include <util.h>

pass1_section_t *pass1_list_first = NULL;
pass1_section_t *pass1_list_last = NULL;

pass1_section_t *actual_section = NULL;

typedef union{
    uint8_t byte;
    uint16_t hword;
    uint32_t word;
}val_t;

typedef enum{
    BYTE,
    HWORD,
    WORD
}val_types_t;

static void _pass1(void);
static int format_integer(val_types_t size, val_t *out_val, long int val);
static void append_to_pass1_list(pass1_item_t *x);

static void handle_section(char *section_name);
static inline pass1_section_t *make_new_section(char *section_name);
static inline pass1_section_t *get_current_section(void);
static inline pass1_section_t *is_section_exist(char *section_name);
static inline void append_to_section_list(pass1_section_t *section);

void pass1(void){
    _pass1();
}

static void _pass1(void){
    uint32_t location_counter = 0;

    for(tok_t * t = toklist_first; t != NULL; t = t->next){
        if(t->type == TOKEN_IS_INSTR){

            //create space for copy of line
            char *op_line = (char *) malloc(sizeof(char) * (strlen(t->payload.i->line) + 1));
            char *op_code = (char *) malloc(sizeof(char) * (strlen(t->payload.i->line) + 1));

            if(op_line == NULL || op_code == NULL){
                fprintf(stderr, "Malloc failed!\n");
                exit(EXIT_FAILURE);
            }

            //make copy of line
            strcpy(op_line, t->payload.i->line);
            strcpy(op_code, t->payload.i->line);

            //cut opcode from that line
            for(int i = 0; op_code[i] != '\0'; i++){
                if(op_code[i] == ';'){
                    op_code[i] = '\0';
                }
            }

            //check if instruction have valid args
            if(check_instruction_args(op_line) == 0){
                fprintf(stderr, "Syntax error in instruction '%s' from file '%s' at line %d!\n", op_code, t->fileInfo->name, t->lineNumber);
                exit(EXIT_FAILURE);
            }

            //create structure for pass1_item
            pass1_item_t *x = (pass1_item_t *)malloc(sizeof(pass1_item_t));

            if(x == NULL){
                fprintf(stderr, "Malloc failed!\n");
                exit(EXIT_FAILURE);
            }

            //fill data into pass1_item
            x->token = t;
            x->location = location_counter;
            x->type = TYPE_INSTRUCTION;
            x->payload.i = t->payload.i;
            x->next = NULL;
            x->prev = NULL;

            //count location for next instruction
            unsigned int actual_size;

            if(get_instruction_size(t->payload.i, &actual_size) == 0){
                fprintf(stderr, "Error in ISA library! errno %d\n", get_isalib_errno());
                exit(EXIT_FAILURE);
            }

            location_counter += actual_size;
            append_to_pass1_list(x);

            //free memory agin
            free(op_line);
            free(op_code);
        }
        else if(t->type == TOKEN_IS_LABEL){
            //copy label and split ':'
            char *label_name = (char *) malloc(strlen(t->payload.l->line) + 1);
            strcpy(label_name, t->payload.l->line);
            for(int i = 0; label_name[i] != '\0'; i++){
                if(label_name[i] == ':') label_name[i] = '\0';
            }

            //make new symbol in table
            new_symbol(label_name, location_counter, STYPE_RELOCATION, t, (void *)get_current_section());

            //cleanup
            free(label_name);
        }
        else if(t->type == TOKEN_IS_PSEUD){

            //create space for copy of line
            char *cmd = (char *) malloc(sizeof(char) * (strlen(t->payload.p->line) + 1));

            if(cmd == NULL){
                fprintf(stderr, "Malloc failed!\n");
                exit(EXIT_FAILURE);
            }

            //make copy of line
            strcpy(cmd, t->payload.p->line);

            //cut opcode from that line
            for(int i = 0; cmd[i] != '\0'; i++){
                if(cmd[i] == ';'){
                    cmd[i] = '\0';
                }
            }

            if(strcmp(cmd, ".ORG") == 0){
                //get position of section name
                char *arg = cmd + strlen(".ORG") + 1;
                val_t new_location;

                if(format_integer(WORD, &new_location, convert_to_int(arg)) == 1){
                    location_counter = new_location.word;
                }
                else{
                    fprintf(stderr, "Syntax error in pseudo '%s' from file '%s' at line %d! Argument is too large!\n", cmd, t->fileInfo->name, t->lineNumber);
                    exit(EXIT_FAILURE);
                }

            }
            else if(strcmp(cmd, ".SECTION") == 0){
                //get position of section name
                char *arg = cmd + strlen(".SECTION") + 1;

                pass1_section_t *s = get_current_section();

                if(s == NULL){
                    handle_section(arg);
                }
                else{
                    s->last_location_counter = location_counter;
                    handle_section(arg);
                    s = get_current_section();
                    location_counter = s->last_location_counter;
                }
            }
            else if(strcmp(cmd, ".CONS") == 0){
                char * cons_name = NULL;    //do not free them!
                char * cons_value = NULL;
                unsigned int x = 0;
                val_t val;

                //put x to "point" to next string in cmd
                x = (unsigned int)strlen(cmd);
                cons_name = &(cmd[++x]);

                //put x to "point" to next string in cmd
                x += (unsigned int)strlen(cons_name);
                cons_value = &(cmd[++x]);

                //convert value into integer
                if(format_integer(WORD, &val, convert_to_int(cons_value)) == 0){
                    fprintf(stderr, "Syntax error in pseudo '%s' from file '%s' at line %d! Argument is too large!\n", cmd, t->fileInfo->name, t->lineNumber);
                    exit(EXIT_FAILURE);
                }

                //add this symbol into symbol table
                new_symbol(cons_name, val.word, STYPE_ABSOLUTE, t, (void *)get_current_section());
            }
            else if(strcmp(cmd, ".DAT_W") == 0){
                //get position of first argument
                char *arg = cmd + strlen(".DAT_W") + 1;

                //count all given arguments
                unsigned int arg_count = 0;
                for(int i = 0; t->payload.p->line[i] != '\0'; i++){
                    if(t->payload.p->line[i] == ';'){
                        arg_count++;
                    }
                }

                //malloc needed space
                pass1_item_t *x = (pass1_item_t *)malloc(sizeof(pass1_item_t));
                blob_t *b = (blob_t *)malloc(sizeof(blob_t));
                uint8_t *b_data = (uint8_t *)malloc(sizeof(uint8_t) * arg_count * 4);

                if(x == NULL || b == NULL || b_data == NULL){
                    fprintf(stderr, "Malloc failed!\n");
                    exit(EXIT_FAILURE);
                }

                //iterate over all arguments
                for(unsigned int i = 0; i < arg_count; i++){
                    val_t val;

                    //convert value into integer
                    if(format_integer(WORD, &val, convert_to_int(arg)) == 0){
                        fprintf(stderr, "Syntax error in pseudo '%s' from file '%s' at line %d! Argument is too large!\n", cmd, t->fileInfo->name, t->lineNumber);
                        exit(EXIT_FAILURE);
                    }

                    //fill data
                    *(b_data + 4*i) = (uint8_t)(val.word & 0xFF);
                    *(b_data + 4*i + 1) = (uint8_t)((val.word >> 8) & 0xFF);
                    *(b_data + 4*i + 2) = (uint8_t)((val.word >> 16) & 0xFF);
                    *(b_data + 4*i + 3) = (uint8_t)((val.word >> 24) & 0xFF);

                    //get next argument
                    arg = arg + strlen(arg) + 1;
                }

                //prepare blob
                b->blob_len = arg_count * 4;
                b->blob_data = b_data;

                //preprase pass1_item
                x->token = t;
                x->location = location_counter;
                x->type = TYPE_BLOB;
                x->payload.b = b;
                x->next = NULL;
                x->prev = NULL;

                //insert it into list
                append_to_pass1_list(x);

                //update location counter
                location_counter += arg_count * 4;
            }
            else if(strcmp(cmd, ".DAT_H") == 0){
                //get position of first argument
                char *arg = cmd + strlen(".DAT_H") + 1;

                //count all given arguments
                unsigned int arg_count = 0;
                for(int i = 0; t->payload.p->line[i] != '\0'; i++){
                    if(t->payload.p->line[i] == ';'){
                        arg_count++;
                    }
                }

                //malloc needed space
                pass1_item_t *x = (pass1_item_t *)malloc(sizeof(pass1_item_t));
                blob_t *b = (blob_t *)malloc(sizeof(blob_t));
                uint8_t *b_data = (uint8_t *)malloc(sizeof(uint8_t) * arg_count * 2);

                if(x == NULL || b == NULL || b_data == NULL){
                    fprintf(stderr, "Malloc failed!\n");
                    exit(EXIT_FAILURE);
                }

                //iterate over all arguments
                for(unsigned int i = 0; i < arg_count; i++){
                    val_t val;

                    //convert value into integer
                    if(format_integer(HWORD, &val, convert_to_int(arg)) == 0){
                        fprintf(stderr, "Syntax error in pseudo '%s' from file '%s' at line %d! Argument is too large!\n", cmd, t->fileInfo->name, t->lineNumber);
                        exit(EXIT_FAILURE);
                    }

                    //fill data
                    *(b_data + 2*i) = (uint8_t)(val.hword & 0xFF);
                    *(b_data + 2*i + 1) = (uint8_t)((val.hword & 0xFF00) >> 8);

                    //get next argument
                    arg = arg + strlen(arg) + 1;
                }

                //prepare blob
                b->blob_len = arg_count * 2;
                b->blob_data = b_data;

                //preprase pass1_item
                x->token = t;
                x->location = location_counter;
                x->type = TYPE_BLOB;
                x->payload.b = b;
                x->next = NULL;
                x->prev = NULL;

                //insert it into list
                append_to_pass1_list(x);

                //update location counter
                location_counter += arg_count * 2;
            }
            else if(strcmp(cmd, ".DAT_B") == 0){
                //get position of first argument
                char *arg = cmd + strlen(".DAT_B") + 1;

                //count all given arguments
                unsigned int arg_count = 0;
                for(int i = 0; t->payload.p->line[i] != '\0'; i++){
                    if(t->payload.p->line[i] == ';'){
                        arg_count++;
                    }
                }

                //malloc needed space
                pass1_item_t *x = (pass1_item_t *)malloc(sizeof(pass1_item_t));
                blob_t *b = (blob_t *)malloc(sizeof(blob_t));
                uint8_t *b_data = (uint8_t *)malloc(sizeof(uint8_t) * arg_count);

                if(x == NULL || b == NULL || b_data == NULL){
                    fprintf(stderr, "Malloc failed!\n");
                    exit(EXIT_FAILURE);
                }

                //iterate over all arguments
                for(unsigned int i = 0; i < arg_count; i++){
                    val_t val;

                    //convert value into integer
                    if(format_integer(BYTE, &val, convert_to_int(arg)) == 0){
                        fprintf(stderr, "Syntax error in pseudo '%s' from file '%s' at line %d! Argument is too large!\n", cmd, t->fileInfo->name, t->lineNumber);
                        exit(EXIT_FAILURE);
                    }

                    //fill data
                    *(b_data + i) = val.byte;

                    //get next argument
                    arg = arg + strlen(arg) + 1;
                }

                //prepare blob
                b->blob_len = arg_count;
                b->blob_data = b_data;

                //preprase pass1_item
                x->token = t;
                x->location = location_counter;
                x->type = TYPE_BLOB;
                x->payload.b = b;
                x->next = NULL;
                x->prev = NULL;

                //insert it into list
                append_to_pass1_list(x);

                //update location counter
                location_counter += arg_count;
            }
            else if(strcmp(cmd, ".DS") == 0){
                //get pointer to argument string
                char *arg = cmd + strlen(".DS") + 1;

                //get len from pseud
                val_t val;
                uint32_t len;

                if(format_integer(WORD, &val, convert_to_int(arg)) == 0){
                    fprintf(stderr, "Syntax error in pseudo '%s' from file '%s' at line %d! Argument is too large!\n", cmd, t->fileInfo->name, t->lineNumber);
                    exit(EXIT_FAILURE);
                }

                len = val.word;

                //create structure for pass1_item and blob
                pass1_item_t *x = (pass1_item_t *)malloc(sizeof(pass1_item_t));
                blob_t *b = (blob_t *)malloc(sizeof(blob_t));
                uint8_t *b_data = (uint8_t *)malloc(sizeof(uint8_t) * len);

                if(x == NULL || b == NULL || b_data == NULL){
                    fprintf(stderr, "Malloc failed!\n");
                    exit(EXIT_FAILURE);
                }

                //fill blob data
                for(uint32_t i = 0; i < len; i++) *(b_data + i) = 0;


                b->blob_data = b_data;
                b->blob_len = len;

                //fill data into pass1_item
                x->token = t;
                x->location = location_counter;
                x->type = TYPE_BLOB;
                x->payload.b = b;
                x->next = NULL;
                x->prev = NULL;

                //append into list
                append_to_pass1_list(x);

                //update location counter
                location_counter += len;
            }
            else if(strcmp(cmd, ".EXPORT") == 0){
                char * export_name = NULL;
                unsigned int x = 0;

                //get address of argument
                x = (unsigned int) strlen(cmd);
                export_name = &(cmd[++x]);

                //add export into symbol table
                new_symbol(export_name, 0, STYPE_EXPORT, t, (void *)get_current_section());
            }
            else if(strcmp(cmd, ".IMPORT") == 0){
                char * import_name = NULL;
                unsigned int x = 0;

                //get address of argument
                x = (unsigned int) strlen(cmd);
                import_name = &(cmd[++x]);

                //add export into symbol table
                new_symbol(import_name, 0, STYPE_IMPORT, t, (void *)get_current_section());
            }
            else{
                fprintf(stderr, "Syntax error, pass1 doesn't know that label! Label '%s' from: %s at line %d.\n", cmd, t->fileInfo->name, t->lineNumber);
                exit(EXIT_FAILURE);
            }

            //free mem again :) we don't want any leaks :P
            free(cmd);
        }
        else{
            fprintf(stderr, "Token doesn't have valid flag!\n");
            exit(EXIT_FAILURE);
        }

    }
}

void pass1_cleanup(void){
    pass1_section_t *tmp_sec = NULL;
    pass1_section_t *head_sec = pass1_list_first;

    while(head_sec != NULL){
        tmp_sec = head_sec;
        head_sec = head_sec->next;

        pass1_item_t *tmp_item = NULL;
        pass1_item_t *head_item = tmp_sec->first_element;

        while(head_item != NULL){
            tmp_item = head_item;
            head_item = head_item->next;

            if(tmp_item->type == TYPE_BLOB){
                free(tmp_item->payload.b->blob_data);
                free(tmp_item->payload.b);
            }

            free(tmp_item);
        }

        //clean up section itself
        free(tmp_sec->section_name);
        free(tmp_sec);
    }

    return;
}

static int format_integer(val_types_t size, val_t *out_val, long int val){
    switch(size){
        case BYTE:
            if(val >= -128 && val <= 127) out_val->byte = (uint8_t) val;
            else return 0;
            break;
        case HWORD:
            if(val >= -32768 && val <= 32767) out_val->hword = (uint16_t) val;
            else return 0;
            break;
        case WORD:
            if(val >= -2147483648 && val <= 2147483647) out_val->word = (uint32_t) val;
            else return 0;
            break;
        default:
            fprintf(stderr, "Internal code error!\n");
            exit(EXIT_FAILURE);
            break;
    }

    return 1;
}

static void append_to_pass1_list(pass1_item_t *x){
    pass1_section_t * s = get_current_section();

    if(s == NULL){
        fprintf(stderr, "Error! Any section dosn't exist! You have to create at least one!\n");
        exit(EXIT_FAILURE);
    }

    if(s->first_element == NULL){
        s->first_element = x;
        s->last_element = x;
    }
    else{
        s->last_element->next = x;
        x->prev = s->last_element;
        s->last_element = x;
    }
}


/* ---------------------------------------------------------------------
 * Functions to handle sections
 */

static void handle_section(char *section_name){

    pass1_section_t *s = is_section_exist(section_name);

    if(s != NULL){ //section exist
        actual_section = s;
    }
    else{
        s = make_new_section(section_name);
        append_to_section_list(s);
        actual_section = s;
    }

}

static inline pass1_section_t *make_new_section(char *section_name){

    pass1_section_t *new_section = (pass1_section_t *)malloc(sizeof(pass1_section_t));
    char *x = (char *) malloc(sizeof(char) * (strlen(section_name) + 1));

    //check malloc result
    if(new_section == NULL || x == NULL){
        fprintf(stderr, "Malloc failed!\n");
        exit(EXIT_FAILURE);
    }

    //copy path
    strcpy(x, section_name);

    //insert content of the new item
    new_section->section_name = x;
    new_section->last_location_counter = 0;
    new_section->prev = NULL;
    new_section->next = NULL;
    new_section->last_element = NULL;
    new_section->first_element = NULL;

    return new_section;
}

static inline pass1_section_t *get_current_section(void){
    return actual_section;
}

static inline pass1_section_t *is_section_exist(char *section_name){
    for(pass1_section_t *s = pass1_list_first; s != NULL; s = s->next){
        if(strcmp(section_name, s->section_name) == 0){
            return s;
        }
    }
    return NULL;
}

static inline void append_to_section_list(pass1_section_t *section){
    if(pass1_list_first == NULL){
        pass1_list_first = section;
        pass1_list_last = section;
    }
    else{
        pass1_list_last->next = section;
        section->prev = pass1_list_last;
        pass1_list_last = section;
    }
}

/* ---------------------------------------------------------------------
 * Debuging functions.
 */

#ifdef DEBUG
void print_pass1_buffer(void){
    printf("\npass1 buffer: \n");

    if(pass1_list_first == NULL){
        printf("  - Section list is empty\n");
    }
    else{
        for(pass1_section_t *s = pass1_list_first; s != NULL; s = s->next){

            printf("  - Section '%s':\n", s->section_name);
            if(s->first_element == NULL){
                printf("      - List is empty\n");
            }
            else{
                for(pass1_item_t *t = s->first_element; t != NULL; t = t->next){
                    if(t->type == TYPE_INSTRUCTION){
                        printf("      - from %s @ %d \t Addr: 0x%X \t INST '%-30s' \n", t->token->fileInfo->name, t->token->lineNumber, t->location, t->payload.i->line);
                    }
                    else if(t->type == TYPE_BLOB){
                        printf("      - from %s @ %d \t Addr: 0x%X \t BLOB with len of %d bytes\n", t->token->fileInfo->name, t->token->lineNumber, t->location, t->payload.b->blob_len);
                    }
                    else{
                        fprintf(stderr, "Internal error in pass1, unknown pass1_item type!\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }
}
#endif
