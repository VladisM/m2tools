#ifndef SENDER_H_included
#define SENDER_H_included

#include <ldm.h>
#include <termios.h>

#define BAUDRATE B38400

typedef struct{
    unsigned int base_address;
    int emulator;
    char * port;
}sender_info_t;

void send_buffer(sender_info_t *info, ldm_buffer_item_t *b);

#endif
