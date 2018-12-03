#ifndef ERROR_H_included
#define ERROR_H_included

typedef enum{
    ERR_OK = 0,
    ERR_CANT_RELOCATE_INSTRUCTION,
    ERR_FILE_NOT_GIVEN,
    ERR_MULTIPLE_FILES_GIVEN
} tLdm2MifError;

#define SET_ERROR(n) if(ldm2mif_errno == ERR_OK) ldm2mif_errno = n;

extern tLdm2MifError ldm2mif_errno;
void print_error(void);

#endif
