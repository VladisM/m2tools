/**
 * @file datatypes.h
 * @brief Datatypes for backend
 *
 * This module should contain three typedefs. First is type to hold addresses in
 * your CPU. It has to be large enough to hold complete address. Another one is
 * dedicated to holding smallest addresable unit in your CPU. It is basically
 * CPU width. Last type is for holding instruction words.
 *
 * In addition to these datatypes, there are also functions that are mentioned for
 * dealing with printing these datatypes into strings and converting them back
 * from string representation into actual numbers.
 */

#ifndef DATATYPES_H_included
#define DATATYPES_H_included

#include <stdint.h>
#include <stdbool.h>

#include <utillib/core.h>

/**
 * @brief Type for holding addresses.
 */
typedef uint32_t isa_address_t;

/**
 * @brief Type for holding whole instruction.
 * @note Should be larger or equal than memory_element_t.
 * @note Have to be integer multiple of isa_memory_element_t.
 * @note All instructions have to fit in single instance of this type.
 */
typedef uint32_t isa_instruction_word_t;

/**
 * @brief Smallest addressable unit.
 * @note used for data blobs
 */
typedef uint32_t isa_memory_element_t;

/**
 * @brief Read isa_address_t value from string.
 * @note Complementary to platformlib_write_isa_address().
 * @param s String to be decoded.
 * @param value Pointer where value will be stored.
 * @return true String was successfully decoded and result is stored in *value.
 * @return false String wasn't decoded correctly and *value is not affected.
 */
bool platformlib_read_isa_address(char *s, isa_address_t *value);

/**
 * @brief Read isa_instruction_word_t value from string.
 * @note Complementary to platformlib_write_isa_instruction_word().
 * @param s String to be decoded.
 * @param value Pointer where value will be stored.
 * @return true String was successfully decoded and result is stored in *value.
 * @return false String wasn't decoded correctly and *value is not affected.
 */
bool platformlib_read_isa_instruction_word(char *s, isa_instruction_word_t *value);

/**
 * @brief Read isa_memory_element_t value from string.
 * @note Complementary to platformlib_write_isa_memory_element().
 * @param s String to be decoded.
 * @param value Pointer where value will be stored.
 * @return true String was successfully decoded and result is stored in *value.
 * @return false String wasn't decoded correctly and *value is not affected.
 */
bool platformlib_read_isa_memory_element(char *s, isa_memory_element_t *value);

/**
 * @brief Write isa_address_t value into string.
 * @note Returned pointer have to be allocated using utillib dynmem_malloc() function!
 * @note Complementary to platformlib_read_isa_address().
 * @param value Numerical value to be converted.
 * @return char* Pointer to allocated string representation of value.
 */
char *platformlib_write_isa_address(isa_address_t value);

/**
 * @brief Write isa_instruction_word_t value into string.
 * @note Returned pointer have to be allocated using utillib dynmem_malloc() function!
 * @note Complementary to platformlib_read_isa_instruction_word().
 * @param value Numerical value to be converted.
 * @return char* Pointer to allocated string representation of value.
 */
char *platformlib_write_isa_instruction_word(isa_instruction_word_t value);

/**
 * @brief Write isa_memory_element_t value into string.
 * @note Returned pointer have to be allocated using utillib dynmem_malloc() function!
 * @note Complementary to platformlib_read_isa_memory_element().
 * @param value Numerical value to be converted.
 * @return char* Pointer to allocated string representation of value.
 */
char *platformlib_write_isa_memory_element(isa_memory_element_t value);

/**
 * @brief Used to convert instruction to memory elements when linker is generating
 * output ldm file.
 * @param word Input data.
 * @param output Result of conversion.
 * @note Function have to create array at pointer location with sizeof(isa_memory_element_t)
 * for each its cell.
 * @note Array have to be cleaned by calling array_destroy when it is not needed anymore.
 */
void platformlib_convert_isa_word_to_element(isa_instruction_word_t word, array_t **output);

#endif
