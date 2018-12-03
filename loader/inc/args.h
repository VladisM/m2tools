#ifndef ARGS_H_included
#define ARGS_H_included

typedef struct{
    //mandatory
    char port[128];
    char file[128];
    unsigned int base_adress;
    //flags
    int help;
    int version;
    int emulator;
}settings_t;

extern settings_t settings;

int get_args(int argc, char *argv[]);

#endif
