#include "verbose.h"

#include "preprocessor.h"
#include "pass_item.h"
#include "section_table.h"
#include "symbol_table.h"

#include <utillib/core.h>

#include <stdlib.h>
#include <stdio.h>

static void spacer(char *title){
    fprintf(stdout, "\r\n------------------\r\n%s\r\n", title);
}

static void spacer_2(char *title, int num){
    fprintf(stdout, "\r\n------------------\r\n%s %d\r\n", title, num);
}

void verbose_print_preprocessor(queue_t *preprocessor_output){
    CHECK_NULL_ARGUMENT(preprocessor_output);

    spacer("preprocessor");

    for(unsigned int i = 0; i < list_count((list_t*)preprocessor_output); i++){
        preprocessed_token_t *head = NULL;
        string_t *line = NULL;

        list_at((list_t *)preprocessor_output, i, (void *)&head);
        string_init(&line);

        string_printf(line, "Token '%s' from ", head->token);
        string_appendf(line, "%s+%ld", head->origin.filename, head->origin.line_number);

        fprintf(stdout, "%s\r\n", string_get(line));

        string_destroy(line);
    }
}

void verbose_print_pass(int pass){
    spacer_2("pass", pass);

    list_t *section_list = section_table_get_all();

    for(unsigned int section_index = 0; section_index < list_count(section_list); section_index++){
        section_t *head_section = NULL;
        list_at(section_list, section_index, (void *)&head_section);

        list_t *head_section_assigned_symbols = head_section->symbols;
        list_t *head_section_assigned_items = head_section->items;

        fprintf(stdout, "Section '%s'\r\n", head_section->section_name);

        for(unsigned int symbols_index = 0; symbols_index < list_count(head_section_assigned_symbols); symbols_index++){
            symbol_t *head_symbol = NULL;
            list_at(head_section_assigned_symbols, symbols_index, (void *)&head_symbol);

            char * val = platformlib_write_isa_address(head_symbol->value);
            fprintf(stdout, "Symbol '%s' value: %s type: %d\r\n", head_symbol->name, val, head_symbol->type);
            dynmem_free(val);
        }

        for(unsigned int items_index = 0; items_index < list_count(head_section_assigned_items); items_index++){
            pass_item_t *head_item = NULL;
            list_at(head_section_assigned_items, items_index, (void *)&head_item);

            char *address = platformlib_write_isa_address(head_item->address);
            fprintf(stdout, "Item at %s is '", address);


            if(head_item->type == ITEM_BLOB){
                fprintf(stdout, "BLOB");
            }
            else{
                if(pass == 1){
                    for(unsigned int j = 0; j < list_count(head_item->args); j++){
                        preprocessed_token_t *tok = NULL;
                        list_at(head_item->args, j, (void *)&tok);
                        fprintf(stdout, "%s ", tok->token);
                    }
                }
                else if(pass == 2){
                    char *value = platformlib_write_isa_address(head_item->value.instr);
                    fprintf(stdout, "%s", value);
                    dynmem_free(value);
                }
                else{
                    error("Verbose print pass! Can't show anything else than pass 1 or 2!");
                }
            }

            fprintf(stdout, "'\r\n");
            dynmem_free(address);
        }
    }
}

void verbose_print_generate(void){
    spacer("generate");
}
