#include "sender.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>

#include <ldm.h>

static void send_3b_word(unsigned int word);
static void send_4b_word(unsigned int word);
static void send_char(unsigned char data);
static void init_port(sender_info_t * info);
static void close_device(void);

int fd;

static void send_3b_word(unsigned int word){
    send_char((unsigned char)((word >> 16) & 0x000000FF));
    send_char((unsigned char)((word >> 8) & 0x000000FF));
    send_char((unsigned char)(word & 0x000000FF));
}

static void send_4b_word(unsigned int word){
    send_char((unsigned char)((word >> 24) & 0x000000FF));
    send_char((unsigned char)((word >> 16) & 0x000000FF));
    send_char((unsigned char)((word >> 8) & 0x000000FF));
    send_char((unsigned char)(word & 0x000000FF));
}

static void send_char(unsigned char data){
    int x = 0;
    int y = 0;
    write(fd, &data, 1);

    //check return code
    do{
        y = (int)read(fd,&x,1);
    }
    while(y != 1);
    if(x == 0xBB){
        fprintf(stderr, "Error! Something went wrong with comunication.\n");
        exit(EXIT_FAILURE);
    }

}

static void init_port(sender_info_t * info){
    /*
     * Based on example from:
     * https://p5r.uk/blog/2009/linux-serial-programming-example.html
     */

    struct termios new_termios;

    fd = open(info->port, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        printf("Error! Can't open port.\n");
        exit(EXIT_FAILURE);
    }

    atexit(close_device);

    memset(&new_termios, 0, sizeof(new_termios));
    new_termios.c_iflag = IGNPAR;
    new_termios.c_oflag = 0;
    if(info->emulator == 1){
        new_termios.c_cflag = CS8 | CREAD | CLOCAL | HUPCL | CRTSCTS;
    }
    else{
        new_termios.c_cflag = CS8 | CREAD | CLOCAL | HUPCL;
    }
    new_termios.c_lflag = 0;
    new_termios.c_cc[VINTR]    = 0;
    new_termios.c_cc[VQUIT]    = 0;
    new_termios.c_cc[VERASE]   = 0;
    new_termios.c_cc[VKILL]    = 0;
    new_termios.c_cc[VEOF]     = 4;
    new_termios.c_cc[VTIME]    = 0;
    new_termios.c_cc[VMIN]     = 1;
    new_termios.c_cc[VSWTC]    = 0;
    new_termios.c_cc[VSTART]   = 0;
    new_termios.c_cc[VSTOP]    = 0;
    new_termios.c_cc[VSUSP]    = 0;
    new_termios.c_cc[VEOL]     = 0;
    new_termios.c_cc[VREPRINT] = 0;
    new_termios.c_cc[VDISCARD] = 0;
    new_termios.c_cc[VWERASE]  = 0;
    new_termios.c_cc[VLNEXT]   = 0;
    new_termios.c_cc[VEOL2]    = 0;

    if(cfsetispeed(&new_termios, BAUDRATE) != 0){
        fprintf(stderr, "Error! Can't setup port.\n");
        exit(EXIT_FAILURE);
    }
    if(cfsetospeed(&new_termios, BAUDRATE) != 0){
        fprintf(stderr, "Error! Can't setup port.\n");
        exit(EXIT_FAILURE);
    }
    if(tcsetattr(fd, TCSANOW, &new_termios) != 0){
        fprintf(stderr, "Error! Can't setup port.\n");
        exit(EXIT_FAILURE);
    }

}

static void close_device(void){
    close(fd);
}

void send_buffer(sender_info_t *info, ldm_buffer_item_t *b){

    init_port(info);

    ldm_buffer_item_t *p = b;
    unsigned int char_count = 0;
    while(p != NULL){
        char_count++;
        p = p->next;
    }

    char greet[] = {0x55};
    write(fd, &greet, 1);
    int y,x=0;
    do{
        y = (int)read(fd,&x,1);
    }
    while(y != 1);
    if(x != 0xAA){
        fprintf(stderr, "Error! Can't connect to the target.\n");
        exit(EXIT_FAILURE);
    }

    printf("Sending %d bytes, please wait...\n", char_count * 4);

    unsigned int per = char_count / 25; //need bytes and percentage so count * 4 / 100
    unsigned int total = 0;
    unsigned int total_counter = 0;

    printf("Sent: 0%%");
    fflush(stdout);

    send_3b_word(info->base_address);
    send_3b_word(char_count);

    while(b != NULL){
        send_4b_word(b->value);

        for(int i = 0; i<4;i++){
            total_counter++;
            if(total_counter == per){
                total++;
                total_counter = 0;
                printf("\rSent: %d%%", total);
                fflush(stdout);
            }
        }
        b = b->next;
    }

    printf("\nSending done.\n");
}
