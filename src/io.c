#include <io.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <console.h>
#include <gibson.h>
#include <stdio.h>
#include <sys/ioctl.h>

#define ANSI_ESC_MOVE_UP "\033[10000A"
#define ANSI_CLEAR_LINE "\033[K"
#define BAR_COLOR "\033[37m"

static void move_cursor(size_t x, size_t y)
{
  char str[50];
  snprintf(str, sizeof(str), "\033[%ld;%ldH", y, x);
  write(STDOUT_FILENO, str, strlen(str));
}


static inline void get_win_size(size_t* width, size_t* height)
{
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  *height = w.ws_row;
  *width = w.ws_col;
}

static inline void update_cursor(void) 
{
  move_cursor(g_context.cursor_x, g_context.cursor_y);
}


/* 
 * 
 * nm means Normal Mode 
 * 
 * This function handles keystrokes
 * in normal mode.
 *
 */
static void nm_handle_keystroke(char c)
{
  size_t height, width;
  get_win_size(&width, &height);

  switch (c)
  {
    case 'q':
      exit(1);
      break;
    case 'i':
      clear_status_line();
      write_status_line("Insert mode");
      g_context.state |= STATE_INSERT_MODE;
      break;
    case 'l':
      if (g_context.cursor_x < width - 1)
      {
        g_context.cursor_x += 1;
        update_cursor();
      }
      break;
    case 'h':
      if (g_context.cursor_x > 1) 
      {
        g_context.cursor_x -= 1;
        update_cursor();
      }
      break;
    case 'j':
      if (g_context.cursor_y < height - 3)
      {
        g_context.cursor_y += 1;
        update_cursor();
      }
      break;
    case 'k':
      if (g_context.cursor_y > 1) 
      {
        g_context.cursor_y -= 1;
        update_cursor();
      }
  }
}


void clear_screen(void)
{
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
}


void write_status_line(const char* str) 
{
  size_t width, height;
  get_win_size(&width, &height);

  move_cursor(0, height-1);
  write(STDOUT_FILENO, BAR_COLOR, strlen(BAR_COLOR));
  write(STDOUT_FILENO, str, strlen(str));
  move_cursor(g_context.cursor_x, g_context.cursor_y);
}


void clear_status_line(void)
{
  size_t width, height;
  get_win_size(&width, &height);

  move_cursor(0, height-1);
  write(STDOUT_FILENO, ANSI_CLEAR_LINE, strlen(ANSI_CLEAR_LINE));
  move_cursor(g_context.cursor_x, g_context.cursor_y);
}

void handle_keystroke(char c)
{
  if (!(g_context.state & STATE_INSERT_MODE)) 
  {
    nm_handle_keystroke(c);
    return;
  }

  if (!(iscntrl(c))) 
  {
    // Write the character to
    // the screen and increase
    // cursor xpos.
    write(STDOUT_FILENO, &c, 1);
    g_context.cursor_x += 1;
  
    // If the text goes too far
    // to the right, make a newline.
    size_t width, height;
    get_win_size(&width, &height);
    if (g_context.cursor_x > width - 2) 
    {
      g_context.cursor_x = 1;
      g_context.cursor_y += 1;
    }
  }
  else if (c == CC_ESC)
  {
    clear_status_line();
    write_status_line("Normal mode");
    g_context.state &= ~(STATE_INSERT_MODE);
  }
}
