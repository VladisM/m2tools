#ifndef ARGS_H_included
#define ARGS_H_included

typedef struct{
    //mandatory
    char out_file[128];
    char file[128];
    unsigned int base_adress;
    unsigned int size;
    //flags
    int help;
    int version;
}settings_t;

extern settings_t settings;

int get_args(int argc, char *argv[]);

#endif
