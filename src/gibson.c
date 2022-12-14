#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

static struct termios old_term_state;

static void exit_callback(void)
{
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_term_state);
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


int main(int argc, char **argv)
{
    (void)argc; // TEMP: Fix compiler warning about unused variables
    (void)argv; // TEMP: Fix compiler warning about unused variables
    
    atexit(exit_callback);
    init_term();
  
    char c;
    while (read(STDIN_FILENO, &c, 1) && c != 'q') 
    {
      if (!(iscntrl(c))) 
      {
        write(STDOUT_FILENO, &c, 1);
      }
    }
    
    // Flush STDOUT to prevent issues on some prompts/terminal emulators.
    write(STDOUT_FILENO, "\n", 1);
    return 0;
}
