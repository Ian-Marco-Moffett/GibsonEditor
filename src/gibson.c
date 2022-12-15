#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <io.h>
#include <gibson.h>
#include <string.h>


static struct termios old_term_state;
EDITOR_CONTEXT g_context = {
  .lines = VECTOR_INIT,
  .is_init = 0
};

static void exit_callback(void)
{
  cleanup();
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_term_state);
  // Flush STDOUT to prevent issues on some prompts/terminal emulators.
  write(STDOUT_FILENO, "\n", 1);
  clear_screen();
}

static void init_term(void) 
{

  struct termios termios;
  tcgetattr(STDIN_FILENO, &old_term_state);
  tcgetattr(STDIN_FILENO, &termios);

  termios.c_lflag &= ~(ECHO | ICANON | ISIG);
  termios.c_iflag &= ~(IXON | ICRNL | ISTRIP | BRKINT);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios);
}


static void run(void) 
{
  char c;
  while (read(STDIN_FILENO, &c, 1)) 
  {
    handle_keystroke(c);
  }
}


int main(int argc, char **argv)
{
  if (argc < 2) 
  {
    printf("Error: Too few arguments!\n");
    return 1;
  }

  if (access(argv[1], F_OK) == 0) 
  {
    printf("Error: Existing files not supported yet\n");
    return 1;
  }

  atexit(exit_callback);
  init_term();
  
  clear_screen();
  g_context.cursor_x = DEFAULT_CURSOR_POSX;
  g_context.cursor_y = DEFAULT_CURSOR_POSY;
  g_context.editing_fname = argv[1];
  
  // Set to normal mode.
  set_default_status(SN_NORMAL_MODE);

  // Move cursor back to 1,1.
  move_cursor(1, 1);

  // Write the first line number.
  write(STDOUT_FILENO, "1~ ", 3);     // First line.

  // Move the cursor to the default position.
  move_cursor(DEFAULT_CURSOR_POSX, 1);

  run();
  return 0;
}
