#include <io.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <console.h>
#include <gibson.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/ioctl.h>

#define ANSI_ESC_MOVE_UP "\033[10000A"
#define ANSI_CLEAR_LINE "\033[K"
#define BAR_COLOR "\033[37m"

static void push_char(char c, uint8_t do_move_cursor);
static inline void update_cursor(void);


static inline void get_win_size(size_t *width, size_t *height)
{
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  *height = w.ws_row;
  *width = w.ws_col;
}

static LINE *get_line(void)
{
  LINE *ln = NULL;

  // FIXME (could cause issues).
  VECTOR_READ_AT(&g_context.lines, &ln, g_context.cursor_y-1);
  return ln;
}

static void newline(uint8_t update_lines)
{
  push_char('\0', 0);
  g_context.cursor_y += 1;
  
  if (update_lines)
  {
    LINE* newline = malloc(sizeof(LINE));
    VECTOR_PUSH(&g_context.lines, newline);
  }
  
  // Write the line number.
  char linenum_buf[20];
  snprintf(linenum_buf, sizeof(linenum_buf), "\n%ld~ ", VECTOR_ELEMENT_COUNT(g_context.lines));
  write(STDOUT_FILENO, linenum_buf, strlen(linenum_buf));
  
  // Move the cursor away from the
  // line number.
  g_context.cursor_x = DEFAULT_CURSOR_POSX;
  update_cursor();
}


static void clear_line(void)
{
  move_cursor(0, g_context.cursor_y);
  write(STDOUT_FILENO, ANSI_CLEAR_LINE, strlen(ANSI_CLEAR_LINE));
}

static void refresh_line(void)
{
  clear_line();
  LINE *ln = get_line();

  // Write the line number.
  char linenum_buf[20];
  snprintf(linenum_buf, sizeof(linenum_buf), "%ld~", g_context.cursor_y);
  write(STDOUT_FILENO, linenum_buf, strlen(linenum_buf));
  
  // - 1 because xpos - 1 is the position before the character
  // we are erasing.
  //
  // Not doing this will cause this specific line buffer
  // to "float" kinda to the right.
  move_cursor(DEFAULT_CURSOR_POSX, g_context.cursor_y);

  // Redraw the text on this one line.
  for (size_t i = 0; i < VECTOR_ELEMENT_COUNT(ln->chars); ++i) {
    write(STDOUT_FILENO, &ln->chars.elements[i], 1);
  }
 
  update_cursor();
}


static void backspace(void)
{
  size_t real_cursor_x = g_context.cursor_x - DEFAULT_CURSOR_POSX;
  LINE *current_line = get_line();

  volatile char unused;
  if (real_cursor_x > 0)
  {
    --g_context.cursor_x;
    update_cursor();
    write(STDOUT_FILENO, " ", 1);
    update_cursor();
    
    // Pop from the buffer.
    VECTOR_POP_AT(&current_line->chars, (char*)&unused, real_cursor_x - 1);

    refresh_line();
  }
  else if (real_cursor_x == 0 && g_context.cursor_y > 1)
  { 
    // Clear the current line. 
    clear_line();

    // Pop and destroy the old line.
    LINE *old_line;
    VECTOR_POP(&g_context.lines, &old_line);
    VECTOR_DESTROY(&old_line->chars);
    free(old_line);

    // Go up.
    --g_context.cursor_y;

    // Move to end of line.
    LINE *ln = get_line(); 
    g_context.cursor_x = DEFAULT_CURSOR_POSX+(VECTOR_ELEMENT_COUNT(ln->chars));

    // Update cursor.
    update_cursor();
  }
}


static void push_char(char c, uint8_t do_move_cursor)
{
  // Mark buffer as changed.
  g_context.is_buffer_changed = 1;

  // Write the character to
  // the screen and increase
  // cursor xpos.
  write(STDOUT_FILENO, &c, 1);

  if (do_move_cursor) 
  {
    g_context.cursor_x += 1;
  }
  
  // If the text goes too far
  // to the right, make a newline.
  size_t width, height;
  get_win_size(&width, &height);
  if (g_context.cursor_x > width - 2) 
  {
    newline(0);
  }

  LINE *line = get_line();
  VECTOR_PUSH(&line->chars, c);
}

static inline void update_cursor(void) 
{
  move_cursor(g_context.cursor_x, g_context.cursor_y);
}


static void init(void)
{
  move_cursor(g_context.cursor_x, g_context.cursor_y);
  LINE *newline = malloc(sizeof(LINE));
  VECTOR_PUSH(&g_context.lines, newline);
  g_context.is_init = 1;
}

static void write_file(void)
{

  g_context.fp = fopen(g_context.editing_fname, "w");
  size_t lines = VECTOR_ELEMENT_COUNT(g_context.lines);
  for (size_t i = 0; i < lines; ++i) 
  {
    LINE *ln = NULL;
    VECTOR_READ_AT(&g_context.lines, &ln, i);
    fputs(ln->chars.elements, g_context.fp);

    if (i < lines-1) {
      fputs("\n", g_context.fp);
    }
  }

  fclose(g_context.fp);
  g_context.is_buffer_changed = 0;
}

/* Moves cursor right */
static void cursor_move_right(void) 
{
  LINE *line = get_line();
  
  /*
   *  By real_cursor_x I don't mean the 
   *  offset from 1 to n, I mean the offset
   *  from DEFAULT_CURSOR_POSX to n.
   *
   */

  size_t real_cursor_x = g_context.cursor_x - DEFAULT_CURSOR_POSX;
  if (real_cursor_x < VECTOR_ELEMENT_COUNT(line->chars)) 
  {
    g_context.cursor_x += 1;
    update_cursor();
  }
}


static void cursor_move_left(void)
{
  // See the comment in cursor_move_right()
  size_t real_cursor_x = g_context.cursor_x - DEFAULT_CURSOR_POSX;
  if (real_cursor_x > 0) 
  {
    g_context.cursor_x -= 1;
    update_cursor();
  }
}


static void cursor_move_up(void) 
{
  if (g_context.cursor_y > 1) 
  {
    // Ensure there is a \n\0
    push_char('\n', 1);
    push_char('\0', 1);

    // Move up.
    g_context.cursor_y -= 1;
    update_cursor();

    LINE *ln = get_line();
    g_context.cursor_x = DEFAULT_CURSOR_POSX+(VECTOR_ELEMENT_COUNT(ln->chars)-1);     // Moves to the end of the line.
    refresh_line();
  }
}


static void cursor_move_down(void) {
  if (g_context.cursor_y < VECTOR_ELEMENT_COUNT(g_context.lines)) {
    g_context.cursor_y += 1;
    update_cursor();

    LINE *ln = get_line();
    g_context.cursor_x = DEFAULT_CURSOR_POSX+(VECTOR_ELEMENT_COUNT(ln->chars));     // Moves to the end of the line.
    update_cursor();
  }
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
  
  // Cancel changes it quit was requested and another key was pressed.
  if (c != CTRL_KEY('q') && g_context.quit_requested)
  {
    g_context.quit_requested = 0;
  }

  switch (c)
  {
    case 'i':
      set_default_status(SN_INSERT_MODE);
      g_context.state |= STATE_INSERT_MODE;
      break;
    case 'l':
      cursor_move_right(); 
      break;
    case 'h':
      cursor_move_left();
      break;
    case 'j':
      cursor_move_down();
      break;
    case 'k': 
      cursor_move_up();
      break;
    case CTRL_KEY('q'):
      if (g_context.is_buffer_changed && !(g_context.quit_requested)) 
      {
        write_status_line("Changes at risk! Press CTRL+Q again to quit, or any other key to cancel.\n");
        g_context.quit_requested = 1;
        break;
      }

      exit(1);
      break;
    case CTRL_KEY('w'):
      // Write \n\0 to the file.
      push_char('\n', 0);
      push_char('\0', 0);

      // Update the status line and file on disk.
      write_status_line("Wrote %ld line(s) to %s\n", VECTOR_ELEMENT_COUNT(g_context.lines), g_context.editing_fname);
      write_file();
      break;
  }
}


void clear_screen(void)
{
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
}


void write_status_line(const char* fmt, ...) 
{
  char buf[300];

  va_list ap;
  va_start(ap, fmt);

  // Write to buf.
  vsnprintf(buf, sizeof(buf), fmt, ap);

  size_t width, height;
  get_win_size(&width, &height);
 
  move_cursor(0, height-1);
  write(STDOUT_FILENO, BAR_COLOR, strlen(BAR_COLOR));
  write(STDOUT_FILENO, buf, strlen(buf));
  move_cursor(g_context.cursor_x, g_context.cursor_y);

  va_end(ap);
}


void clear_status_line(void)
{
  size_t width, height;
  get_win_size(&width, &height);

  move_cursor(0, height-1);
  write(STDOUT_FILENO, ANSI_CLEAR_LINE, strlen(ANSI_CLEAR_LINE));
  move_cursor(g_context.cursor_x, g_context.cursor_y); 
}

// Cleans up.
void cleanup(void) 
{
  while (!(VECTOR_IS_EMPTY(g_context.lines)))
  {
    LINE *line = NULL;
    VECTOR_POP(&g_context.lines, &line);
    VECTOR_DESTROY(&line->chars);
    free(line);
  }
}

void set_default_status(STATUS_NUMBER status_number)
{
  switch (status_number)
  {
    case SN_NORMAL_MODE: 
      clear_status_line();
      write_status_line("Normal mode - %s\n", g_context.editing_fname);
      break;
    case SN_INSERT_MODE:
      clear_status_line();
      write_status_line("Insert mode - %s\n", g_context.editing_fname);
      break;
  }
}

void move_cursor(size_t x, size_t y) {
  char str[50];
  snprintf(str, sizeof(str), "\033[%ld;%ldH", y, x);
  write(STDOUT_FILENO, str, strlen(str));
}

void handle_keystroke(char c)
{

  if (!(g_context.is_init))
  {
    init();
  }

  if (!(g_context.state & STATE_INSERT_MODE)) 
  {
    nm_handle_keystroke(c);
    return;
  } 

  if (!(iscntrl(c))) 
  { 
    push_char(c, 1);
  }
  else if (c == CC_ESC)
  {
    clear_status_line();
    set_default_status(SN_NORMAL_MODE);
    g_context.state &= ~(STATE_INSERT_MODE);
  }
  else if (c == CC_ENTER)
  {
    newline(1);
  } 
  else if (c == CC_BACKSPACE)
  {
    backspace();
  }
}
