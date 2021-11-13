#ifndef FILELIB_CHECK_STRUCTURE_H_defined
#define FILELIB_CHECK_STRUCTURE_H_defined

typedef void (check_structure_t)(void *input);

check_structure_t check_structure_ldm;
check_structure_t check_structure_obj;
check_structure_t check_structure_sl;

#endif
