#include <util.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

long int convert_to_int(char *l){
    size_t len = strlen(l);

    if(len < 1){
        fprintf(stderr, "Convert int error!\n");
        exit(EXIT_FAILURE);
    }
    else{
        long int temp = 0;
        char *end_ptr = NULL;

        if(l[0] == '0' && l[1] == 'x'){
            temp = strtol(l, &end_ptr, 16);

            if(end_ptr == l){
                fprintf(stderr, "Error when converting string '%s' into integer value!\n", l);
                exit(EXIT_FAILURE);
            }
        }
        else if(l[0] == '0' && l[1] == 'b'){
            char *nptr = l + 2;
            temp = strtol(nptr, &end_ptr, 2);

            if(end_ptr == nptr){
                fprintf(stderr, "Error when converting string '%s' into integer value!\n", l);
                exit(EXIT_FAILURE);
            }
        }
        else{
            temp = strtol(l, &end_ptr, 10);

            if(end_ptr == l){
                fprintf(stderr, "Error when converting string '%s' into integer value!\n", l);
                exit(EXIT_FAILURE);
            }
        }

        return temp;
    }
}

int is_number(char *s){

    for(int i = 0; s[i] != '\0'; i++){

        if(isxdigit(s[i])){
            continue;
        }
        else{
            if(i == 1){
                if(s[i] == 'x'){
                    continue;
                }
                else if(s[i] == 'b'){
                    continue;
                }
                else{
                    return 0;
                }
            }
            else{
                return 0;
            }
        }
    }

    return 1;
}
