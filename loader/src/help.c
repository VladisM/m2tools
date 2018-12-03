#include "help.h"

#include <stdio.h>

void print_version(void){
    printf("loader for MARK-II CPU %s\n", VERSION);
}

void print_help(void){
    const char * help_string = "\
Example usage: loader -b 0x400 -p /dev/ttyUSB0 example.ldm\n\
\n\
        Simple utility to load program into MARK-II using default serial\n\
    bootloader. For more information please see following link:\n\
    https://www.github.com/VladisM/MARK_II\n\
\n\
Arguments:\n\
    -h --help           Print this help.\n\
    -b <address>        Base address, using hex C like syntax, to store source.\n\
                        Loader also perform relocation of the given source to\n\
                        this address.\n\
    -p <port>           Port where MARK-II is connected. For example\n\
                        /dev/ttyUSB0.\n\
       --version        Print version number and exit.\n\
    -e --emulator       Add this option if you are connecting to emulator.\n\
";
    printf("%s", help_string);
}
