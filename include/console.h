#ifndef CONSOLE_H_
#define CONSOLE_H_

#define CTRL_KEY(k) ((k) & 0x1F)


typedef enum
{
  CC_ESC    = 0x1B,
  CC_ENTER  = 0x0D,
} CONSOLE_CODE;


#endif
