#ifndef PLATFORMLIB_PRIVATE_H_included
#define PLATFORMLIB_PRIVATE_H_included

#include <stdbool.h>
#include <utillib/utils.h>

#define ERROR_WRITE(x, ...) error_buffer_write(platformlib_error_buffer, (x), ##__VA_ARGS__)
#define CHECK_INITIALIZED() { if(platformlib_initialized == false){ error("Platform lib is not initialized!"); }}

extern bool platformlib_initialized;
extern error_t *platformlib_error_buffer;

#endif
