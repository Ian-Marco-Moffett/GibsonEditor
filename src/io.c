#include <io.h>
#include <unistd.h>
#include <string.h>

#define ANSI_ESC_CLEAR "\033[2J"
#define ANSI_ESC_MOVE_UP "\033[10000A"

void clear_screen(void) 
{
  write(STDOUT_FILENO, ANSI_ESC_MOVE_UP, strlen(ANSI_ESC_MOVE_UP));
  write(STDOUT_FILENO, ANSI_ESC_CLEAR, strlen(ANSI_ESC_CLEAR));
}
