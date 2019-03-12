#include <stdlib.h>
#include <stdio.h>

#define HELP_STRING "\
Example usage: %s \n\
HELP_STRING of the archiver utility.\n"

static void print_version(void);
static void print_help(char *cmd_name);

int main(int argc, char **argv){
    return 0;
}

static void print_version(void){
    printf("archiver for MARK-II CPU %s\n", VERSION);
}

static void print_help(char *cmd_name){
    printf(HELP_STRING, cmd_name);
}
