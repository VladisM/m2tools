#include "datatypes.h"
#include "assemble.h"
#include "instructions_description.h"
#include "miscellaneous.h"

#define UNUSED(x) (void)x

instruction_signature_t platformlib_instruction_signatures[] = {
    {NULL, 0, 1}
};

static void _error(void){
    error("This is example target that isn't intended for running!");
}

bool platformlib_assemble_instruction(
    char **args,
    int argc,
    bool (*find_symbol_callback)(char *label, void *section, isa_address_t *result),
    void *section,
    isa_instruction_word_t *result
){
    UNUSED(args);
    UNUSED(argc);
    UNUSED(find_symbol_callback);
    UNUSED(section);
    UNUSED(result);
    _error();
    return false;
}

bool platformlib_relocate_instruction(
    isa_instruction_word_t input,
    isa_instruction_word_t *output,
    isa_address_t offset
){
    UNUSED(input);
    UNUSED(output);
    UNUSED(offset);
    _error();
    return false;
}

bool platformlib_retarget_instruction(
    isa_instruction_word_t input,
    isa_instruction_word_t *output,
    isa_address_t target
){
    UNUSED(input);
    UNUSED(output);
    UNUSED(target);
    _error();
    return false;
}

bool platformlib_read_isa_address(char *s, isa_address_t *value){
    UNUSED(s);
    UNUSED(value);
    _error();
    return false;
}

bool platformlib_read_isa_instruction_word(char *s, isa_instruction_word_t *value){
    UNUSED(s);
    UNUSED(value);
    _error();
    return false;
}

bool platformlib_read_isa_memory_element(char *s, isa_memory_element_t *value){
    UNUSED(s);
    UNUSED(value);
    _error();
    return false;
}

char *platformlib_write_isa_address(isa_address_t value){
    UNUSED(value);
    _error();
    return NULL;
}

char *platformlib_write_isa_instruction_word(isa_instruction_word_t value){
    UNUSED(value);
    _error();
    return NULL;
}

char *platformlib_write_isa_memory_element(isa_memory_element_t value){
    UNUSED(value);
    _error();
    return NULL;
}

void platformlib_convert_isa_word_to_element(isa_instruction_word_t word, array_t **output){
    UNUSED(word);
    UNUSED(output);
    _error();
}

bool platformlib_get_instruction_opcode(isa_instruction_word_t word, char **opcode){
    UNUSED(word);
    UNUSED(opcode);
    _error();
    return false;
}
