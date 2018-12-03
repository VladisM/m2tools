#ifndef ERROR_H_included
#define ERROR_H_included

typedef enum{
    ERR_OK = 0,
    ERR_RELOCATION_FAIL,
    ERR_CANT_OPEN_PORT,
    ERR_CANT_SETUP_PORT,
    ERR_CANT_CONNECT,
    ERR_PORT_NOT_GIVEN,
    ERR_FILE_NOT_GIVEN,
    ERR_BASE_NOT_GIVEN,
    ERR_MULTIPLE_FILES_GIVEN,
    ERR_COMUNICATION
}tLoaderError;

#define SET_ERROR(n) if(loader_errno == ERR_OK) loader_errno = n

extern tLoaderError loader_errno;
void print_error(void);

#endif
