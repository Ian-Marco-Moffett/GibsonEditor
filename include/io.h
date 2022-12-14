#ifndef IO_H_
#define IO_H_

#define  _POSIX_C_SOURCE 200809L

#include <stddef.h>
#include <stdio.h>

#define DEFAULT_CURSOR_POSX 8
#define DEFAULT_CURSOR_POSY 1

// Status numbers
typedef enum
{
  SN_NORMAL_MODE,
  SN_INSERT_MODE
} STATUS_NUMBER;


void set_default_status(STATUS_NUMBER status_number);
void handle_keystroke(char c);
void write_status_line(const char* fmt, ...);
void clear_status_line(void);
void move_cursor(size_t x, size_t y);
void clear_screen(void);
void read_in_file(FILE* fp);

#endif
