#include "help.h"

#include <stdio.h>

void print_version(void){
    printf("ldm2mif for MARK-II CPU %s\n", VERSION);
}

void print_help(void){
    const char * help_string = "\
Example usage: ldm2mif example.ldm\n\
\n\
        This is simple utility to convert load module (.ldm) file from linker\n\
    into memory inicialization file for Quartus II. Default address range of\n\
    output file is 2^8.\n\
\n\
Arguments:\n\
    -h --help           Print this help.\n\
    -o <file>           Output MIF name. If not specified default name\n\
                        will be used.\n\
    -r <address>        Relocate source. Addjust immediate addresses of these\n\
                        instructions that use relative addresing using labels.\n\
                        You have to specify <address> in hex where the code\n\
                        will be stored. Default value is 0x000000.\n\
    -s <size>           Size of memory, default value is 8. Memory range is\n\
                        from 0 to 2^<size>.\n\
       --version        Print version number and exit.\n\
";
    printf("%s", help_string);
}
