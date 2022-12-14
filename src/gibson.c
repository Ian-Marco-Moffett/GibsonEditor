#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <io.h>
#include <gibson.h>

static struct termios old_term_state;
EDITOR_CONTEXT g_context = {0};

static void exit_callback(void)
{
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
    (void)argc; // TEMP: Fix compiler warning about unused variables
    (void)argv; // TEMP: Fix compiler warning about unused variables
    
    atexit(exit_callback);
    init_term();
  
    clear_screen();
    g_context.cursor_x = 1;
    g_context.cursor_y = 1;

    write_status_line("Normal mode\n");

    run();
    return 0;
}
