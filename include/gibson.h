#ifndef GIBSON_H_
#define GIBSON_H_

#include <stddef.h>
#include <stdint.h>

#define STATE_INSERT_MODE (1 << 0)

typedef struct
{
  size_t cursor_x;
  size_t cursor_y;

  uint8_t state;
} EDITOR_CONTEXT;

extern EDITOR_CONTEXT g_context;

#endif
