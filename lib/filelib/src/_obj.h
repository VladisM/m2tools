#ifndef FILELIB_PRIVATE_OBJ_H_included
#define FILELIB_PRIVATE_OBJ_H_included

#include "obj.h"

/**
 * @brief Load obj file from string, used in static library to avoid creating tmp files.
 *
 * @param input String from static library.
 * @param f Object to store output.
 * @param filename Name of origin file for error messages.
 */
bool obj_load_string(string_t *input, obj_file_t **f, char *filename);

/**
 * @brief Write obj file object into string. Used in static libraries to avoid creating tmp files.
 *
 * @param output String to write result into.
 * @param f Structure with object file.
 */
void obj_write_string(obj_file_t *f, string_t **output);

#endif
