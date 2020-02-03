/**
 * @file obj.h
 *
 * @brief Header file for library that deal with tasks related with object files.
 *
 * @author Bc. Vladislav Mlejnecký <v.mlejnecky@seznam.cz>
 * @date 15.06.2019
 *
 * @note This file is part of m2tools project.
 *
 * @todo Add example of ussage and describe obj group more.
 */

#ifndef OBJ_H_included
#define OBJ_H_included

#include <stdint.h>
#include <stdbool.h>

#include <isa.h>

/**
 * @defgroup obj Object files library
 *
 * @addtogroup obj
 *
 * @{
 */

/**
 * @brief Symbol type.
 */
typedef enum{
    SYMBOL_EXPORT = 0,  /**< @brief Symbol is exported from current section. */
    SYMBOL_IMPORT       /**< @brief Symbol is imported into current section. */
}symbol_type_t;

/**
 * @brief Enum type used to hold information about payload in data_symbol_t struct.
 */
typedef enum{
    DATA_IS_BLOB = 0,   /**< @brief Stored payload in blob struct, datablob_t. */
    DATA_IS_INST        /**< @brief Stored payload in instruction struct, tInstruction. */
}data_symbol_type_t;

/**
 * @brief Structure to hold data blob.
 */
typedef struct{
    uint8_t *payload;    /**< @brief Pointer to first byte of payload. */
    unsigned int lenght; /**< @brief Hold information about how long payload is. */
}datablob_t;

/**
 * @brief Structure to hold information about one elementary data symbol.
 *
 * Data symbol can be two types, blob or inst. Inst mean instruction, and is
 * more specified by pointer to tInstruction struct in file isa.h. Blob is
 * binary blob, it simple data payload without any big meaning.
 *
 * All data symbols in section together form a double linked list.
 */
typedef struct data_symbol_s{
    struct data_symbol_s *next;    /**< @brief Pointer to the next symbol in the list. */
    struct data_symbol_s *prev;    /**< @brief Pointer to the previous symbol in the list. */
    union{
        datablob_t *blob;          /**< @brief Correct pointer when type == DATA_IS_BLOB. */
        tInstruction *inst;        /**< @brief Correct pointer when type == DATA_IS_INST. */
    }payload;                      /**< @brief Union that hold pointer to payload. */
    isa_address_t address;              /**< @brief Address of symbol in memory. This isn't absolute addres, it is rather relative to the beggining of the section.*/
    data_symbol_type_t type;       /**< @brief Type of this symbol. */
    bool relocation;            /**< @brief Store information about relocation. 0 if should not be relocated, 1 otherwise. */
    bool special;               /**< @brief Store information about argument, if is special symbol. 1 it is, 0 it isn't. */
}data_symbol_t;

/**
 * @brief Structure to hold information about one special symbol.
 *
 * Special symbols are used by linker, when creating executable binary.
 * All symbols tohether form and double linked list. One list for every section
 * of object file. This double linked list is called special symbol table.
 */
typedef struct spec_symbol_s{
    char *name;                     /**< @brief Name of the symbol. */
    struct spec_symbol_s *next;     /**< @brief Pointer to the next symbol in list.  */
    struct spec_symbol_s *prev;     /**< @brief Pointer to the previous symbol in the list. */
    isa_address_t value;                 /**< @brief Value for this symbol.*/
    symbol_type_t type;             /**< @brief Type of the symbol, can be exported or imported to the section.*/
}spec_symbol_t;

/**
 * @brief Structure to hold one section of the object file.
 *
 * All section together in object file, form an double linked list. Each section have
 * its own special symbol table, and its own data symbols (can be instructions or
 * blobs).
 *
 * Section have names, they are used by linker for linking them.
 *
 * They can also be section that dosn't have any symbols exported or inported, and
 * they are valid too. In this case both pointers spec_symbol_first and spec_symbol_last
 * are pointing to NULL.
 */
typedef struct section_s{
    char *section_name;                 /**< @brief Name of this section, name is given by command .SECTION <name> in asm file. */
    struct section_s *next;             /**< @brief Pointer to the next section in object file. */
    struct section_s *prev;             /**< @brief Pointer to the previous sections in boject file. */
    spec_symbol_t *spec_symbol_first;   /**< @brief Pointer to the first element of special symbol table. */
    spec_symbol_t *spec_symbol_last;    /**< @brief Pointer to the last element of the special symbol table. */
    data_symbol_t *data_first;          /**< @brief Pointer to the first element of data stream. */
    data_symbol_t *data_last;           /**< @brief Pointer to the last element of data stream. */
}section_t;


/**
 * @brief Struct to hold one complete object file.
 *
 * Object file have some properties like target_arch_name and object_file_name.
 * Also it is supposed to be connected into double linked list of object files.
 * This feature can be used by linker or archiver.
 *
 * Object file consist of list of sections connected into (well ... again) double
 * linked list (yeah, I love double linked lists).
 */
typedef struct obj_file_s{
    char *object_file_name;     /**< @brief Name of this object file. Usually same as filename.*/
    char *target_arch_name;     /**< @brief Name of target architecture, library have to be compiled for same target. */
    section_t *first_section;   /**< @brief Pointer to first section in list. */
    section_t *last_section;    /**< @brief Pointer to last section in the list. */
    struct obj_file_s *next;    /**< @brief Pointer to the next object file. */
    struct obj_file_s *prev;    /**< @brief Pointer to the last object file in the list. */
}obj_file_t;

/**
 * @brief Enum type for errors in objlib.
 */
typedef enum{
    OBJRET_OK = 0,                      /**< @brief Everything is OK. :) */
    OBJRET_BROKEN_OBJ,                  /**< @brief Object file is broken, excepted something different from what we get. */
    OBJRET_NULL_PTR,                    /**< @brief Pointer in function arg is NULL. */
    OBJRET_MALLOC_FAIL,                 /**< @brief Failed to allocate memory. */
    OBJRET_BROKEN_SECTION,              /**< @brief Section is incosistent.*/
    OBJRET_FOPEN_ERROR,                 /**< @brief Failed to open file. */
    OBJRET_INTERNAL_ERR,                /**< @brief Not specified error, may occur in underlying library. */
    OBJRET_BROKEN_FILE,                 /**< @brief Structure of the object file in corrupted. */
    OBJRET_SECTION_EXIST_ALREADY,       /**< @brief This section already exist in object file. You should connet them before adding it.*/
    OBJRET_WRONG_ARCH,                  /**< @brief Object file is intended for another target than library. */
    OBJRET_WRONG_ARG                    /**< @brief Given arguments for fuction are in wrong format. Maybe some pointer have to be NULL and it doesn't? */
}obj_file_err_t;

/**
 * @brief Clear variable holding error code.
 */
void clear_objlib_errno(void);

/**
 * @brief Get error code.
 *
 * @return One of the obj_file_err_t value.
 */
obj_file_err_t get_objlib_errno(void);

/**
 * @brief Clear object file.
 *
 * Object file is full of pointer, pointing to pointeres, pointing to another pointeres,
 * that finally, point to chars arrays, witch is ... pointers. This all is dynamically
 * allocated memory and you have to free it, you this function to do so. :)
 *
 * @param o Pointer to object file to clean.
 */
void free_object_file(obj_file_t *o);

/**
 * @brief Clear section in object file.
 *
 * This function should not be exported ... but sections as defined by section_t in this
 * module are used internaly in linker.
 *
 * @warning Use it only on deep copy of sections from object file. Otherwise, you will break up
 * object file structure!
 */
void free_obj_section(section_t *section);

/**
 * @brief Clear special symbol in object file.
 *
 * This function should not be exported ... but special symbols as defined by spec_symbol_t in this
 * module are used internaly in linker.
 *
 * @warning Use it only on deep copy of sections from object file. Otherwise, you will break up
 * object file structure!
 */
void free_obj_spec_symbol(spec_symbol_t *symbol);

/**
 * @brief Clear data symbol in object file.
 *
 * This function should not be exported ... but data symbols as defined by data_symbol_t in this
 * module are used internaly in linker.
 *
 * @warning Use it only on deep copy of sections from object file. Otherwise, you will break up
 * object file structure!
 */
void free_obj_data_symbol(data_symbol_t *symbol);

/**
 * @brief Load object file from string buffer into structure.
 *
 * @param s String that hold content of object file.
 * @param o Pointer for struct where object file will be loaded into.
 *
 * @return true if ok, false if failed
 *
 * @note param o have to point to NULL ( *o == NULL ), correct struct will be
 * created by calling this function.
 *
 * @note This function is intended mainly for using in linker or archiver. Object file is
 * generaly only text file in special format, and therefore can be stored in string.
 */
bool obj_load_from_string(char *s, obj_file_t **o);

/**
 * @brief Load object file from regular file on the disk.
 *
 * @param filename Path with file that will be loaded.
 * @param o Pointer for struct where object file will be loaded into.
 *
 * @return true if ok, false if failed
 *
 * @note param o have to point to NULL ( *o == NULL ), correct struct will be
 * created by calling this function.
 */
bool obj_load_from_file(char *filename, obj_file_t **o);

/**
 * @brief Write object file into string.
 *
 * @param s Pointer to string where object will be printed out.
 * @param o Pointer to object that will be written out.
 *
 * @return true if ok, false if failed
 *
 * @note param s have to point to NULL ( *s == NULL ), memory will be allocated by
 * this function. Don't forget to free it! ;)
 *
 * @note This function is intended mainly for using in linker or archiver. Object file is
 * generaly only text file in special format, and therefore can be stored in string.
 */
bool obj_write_to_string(char **s, obj_file_t *o);

/**
 * @brief Write object file into regular file.
 *
 * @param filename Path with file name where object file will be written to.
 * @param o Pointer to object that will be written out.
 *
 * @return true if ok, false if failed
 */
bool obj_write_to_file(char *filename, obj_file_t *o);

/**
 * @brief Create new structure for object file.
 *
 * @param object_file_name String with name of object file.
 * @param o Pointer to pointer to object struct.
 *
 * @note o have to be pointing to NULL. ( *o == NULL ). Don't forget ot
 * free this struct by calling free_object_file().
 *
 * @return true if ok, false if failed
 */
bool new_obj(char * object_file_name, obj_file_t **o);

/**
 * @brief Create new section.
 *
 * @param secion_name Name of the section that will be created.
 * @param s Pointer to pointer to section struct.
 *
 * @note s have to be pointing to NULL. ( *s == NULL ). This structure is deallocated
 * with calling free_object_file on the object where this section is.
 *
 * @return true if ok, false if failed
 */
bool new_section(char *secion_name, section_t **s);

/**
 * @brief Create new special symbol.
 *
 * @param name Name of the symbol.
 * @param value Value of the symbol.
 * @param type Type of the symbol, export or import. See symbol_type_t.
 * @param s Pointer to pointer to struct where symbol will be created.
 *
 * @note s have to be pointing to NULL. ( *s == NULL ). This structure is deallocated
 * with calling free_object_file on the object where this section is.
 *
 * @return true if ok, false if failed
 */
bool new_spec_symbol(char *name, isa_address_t value, symbol_type_t type, spec_symbol_t **s);

/**
 * @brief Create new data symbl.
 *
 * @param address Address where this symbol will be stored relative to the beggining of section.
 * @param type Type of the data symbol, can be instruction or blob see data_symbol_type_t.
 * @param payload_ptr Pointer (void *), that will point to the structure, it can be instance of structure from isa, or datablob_t.
 * @param d Pointer to pointer to struct where symbol will be created.
 *
 * @note d have to be pointing to NULL. ( *d == NULL ). This structure is deallocated
 * with calling free_object_file on the object where this section is.
 *
 * @return true if ok, false if failed
 */
bool new_data_symbol(isa_address_t address, data_symbol_type_t type, void *payload_ptr, data_symbol_t **d);

/**
 * @brief Create new instance of datablob_struct.
 *
 * @param lenght Bytes needed for payload.
 * @param b Pointer to pointer to struct where blob will be created.
 *
 * @return true if ok, false if failed
 *
 * @note b have to be pointing to NULL. ( *b == NULL ). This structure is deallocated
 * with calling free_object_file on the object where this section is.
 *
 * This function dosn't fill your blob with any data, you have to do it at yourself,
 * see datablob_t for details about structure members.
 */
bool new_blob(unsigned int lenght, datablob_t **b);

/**
 * @brief Append section into object file.
 *
 * This function will connect section s into double linked list in object o.
 *
 * @param o Object file structure.
 * @param s Section to connect.
 *
 * @return true if ok, false if failed
 */
bool append_section_to_obj(obj_file_t *o, section_t *s);

/**
 * @brief Append special symbol into section.
 *
 * This function connect special symbol at the end of the double linked list in the section.
 *
 * @param section Section to insert special symbol.
 * @param symbol Symbol to insert.
 *
 * @return true if ok, false if failed
 */
bool append_spec_symbol_to_section(section_t *section, spec_symbol_t *symbol);

/**
 * @brief Append data symbol to section.
 *
 * This function connect data symbol at the end of the double linked list in the section.
 *
 * @param section Section to insert data symbol.
 * @param data Data symbol that will be inserted.
 *
 * @return true if ok, false if failed
 */
bool append_data_symbol_to_section(section_t *section, data_symbol_t *data);

/**
 * @}
 */

#endif
