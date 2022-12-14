#ifndef IO_H_
#define IO_H_

#include <stddef.h>


void handle_keystroke(char c);
void write_status_line(const char* str);
void clear_status_line(void);
void clear_screen(void);

#endif
