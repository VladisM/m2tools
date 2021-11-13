#include "test_common.h"

#include <stdio.h>
#include <stdbool.h>

bool requested_arguments_exact(int argc, int requested){
    if(argc != requested){
        if(argc > requested){
            printf("Too much arguments given for this test!\r\n");
        }
        else{
            printf("Not enough arguments given for this test!\r\n");
        }
        return false;
    }
    else{
        return true;
    }
}

bool requested_arguments_more(int argc, int requested){
    if(argc < requested){
        printf("Not enough arguments given for this test!\r\n");
        return false;
    }
    else{
        return true;
    }
}
