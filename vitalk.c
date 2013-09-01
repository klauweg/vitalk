#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include "vito_io.h"

// Globals:
int frame_debug = 0;

main()
{
  unsigned char vitomem[20];
  int res;
   opentty("/dev/ttyUSB0");
   vito_init();

  
frame_debug = 1;
  vito_read(0x5525, 2, vitomem);
  res = vitomem[0] << 8;
  res+= vitomem[1];
  
fprintf(stderr, "VITOMEM: %x\n", res);
  
  vito_close();
  closetty();
}

   