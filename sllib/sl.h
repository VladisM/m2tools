/**
 * @file sl.h
 *
 * @brief Library for dealing with static library files.
 *
 * @todo Add example and some description
 */

#ifndef SL_H_included
#define SL_H_included

#include <stdbool.h>

#include <obj.h>

/**
 * @defgroup sl Library for static library
 *
 * @addtogroup sl
 *
 * @{
 */

/**
 * @brief Structure for holding static library.
 *
 * Static library is quiet simle object. It consist of string name, string
 * target arch name, and double-linked list of object files.
 */
typedef struct{
    char *library_name;         /**< @brief Name of the library. */
    char *target_arch_name;     /**< @brief Name of the library target architecture. */
    obj_file_t *first_obj;      /**< @brief Pointer to first object file in library. They are in double-linked list. */
    obj_file_t *last_obj;       /**< @brief Pointer to last object file in library. They are in double-linked list. */
}static_library_t;

/**
 * @brief Enum with error types.
 */
typedef enum{
    SLRET_OK = 0,               /**< @brief Everything is ok :) */
    SLRET_NULL_PTR,             /**< @brief One of the argument is pointing to NULL and it shouldn't. */
    SLRET_BROKEN_SL,            /**< @brief File is corrupted! */
    SLRET_OBJ_EXIST_ALREADY,    /**< @brief This object file already exist in library. */
    SLRET_MALLOC_FAIL,          /**< @brief Failed to allocate memory. Dude, this is serious problem! */
    SLRET_INTERN_ERR,           /**< @brief Internal error, probably underlying layers went crazy... */
    SLRET_LIB_EXIST_ALREADY,    /**< @brief Pointer in argument is point to !=NULL and it should point to NULL. */
    SLRET_CANT_CREATE_SL,       /**< @brief Failed to create library for whatever reason. */
    SLRET_TMP_WRITE_ERR,        /**< @brief Failed to write object file into temp string buffer. */
    SLRET_CANT_OPEN_SL,         /**< @brief Failed to open library file. */
    SLRET_WRONG_ARCH            /**< @brief Library is intended for another target. */
}sl_err_t;

/**
 * @brief Clear variable holding error code.
 */
void clear_sllib_errno(void);

/**
 * @brief Get error code.
 *
 * @return One of the sl_err_t value.
 */
sl_err_t get_sllib_errno(void);

/**
 * @brief Clear dynamic memory allocated by library structure.
 *
 * @param lib Pointer to library to clear.
 */
void free_sl(static_library_t *lib);

/**
 * @brief Load library from file.
 *
 * @param filename Path to file that will be loaded.
 * @param lib Pointer to pointer to struct where library will be loaded.
 *
 * @return true if ok, false if failed
 *
 * @note lib have to point to NULL (*lib == NULL && lib != NULL)
 */
bool sl_load(char *filename, static_library_t **lib);

/**
 * @brief Write library into file.
 *
 * @param filename Path to file where library will be written to.
 * @param lib Pointer to library struct that will be written.
 *
 * @return true if OK, false if failed
 */
bool sl_write(char *filename, static_library_t *lib);

/**
 * @brief Create new instane of library struct and prefill it with correct data.
 *
 * Use append_objfile_to_sl() function to populate your library.
 *
 * @param lib_name Name of the library, usually same as filename.
 * @param lib Pointer to pointer to struct where library will be stored.
 *
 * @return true if OK, false if failed
 *
 * @note lib have to point to NULL (*lib == NULL && lib != NULL)
 *
 * @note New structure will be created at heap, using malloc, at the end of
 * the program you have to free it by calling free_sl() function.
 */
bool new_sl(char *lib_name, static_library_t **lib);

/**
 * @brief Function that append object file into library.
 *
 * This function will append given instance of  object file into existing library.
 *
 * @param o Pointer to object file that will be added.
 * @param lib Pointer to library.
 *
 * @return true if OK, false if failed
 */
bool append_objfile_to_sl(obj_file_t *o, static_library_t *lib);

/**
 * @}
 */

#endif
