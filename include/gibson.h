#ifndef GIBSON_H_
#define GIBSON_H_

#include <stddef.h>
#include <stdint.h>
#include <vector.h>
#include <stdio.h>

#define STATE_INSERT_MODE (1 << 0)


typedef struct {
  VECTOR_TYPE(char) chars;
} LINE;


typedef struct
{
  size_t cursor_x;
  size_t cursor_y;

  uint8_t state;
  VECTOR_TYPE(LINE*) lines;

  uint8_t is_init : 1;
  uint8_t is_buffer_changed : 1;
  uint8_t quit_requested : 1;
  const char* editing_fname;
  FILE* fp;
} EDITOR_CONTEXT;

void cleanup(void);

extern EDITOR_CONTEXT g_context;

#endif
