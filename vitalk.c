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
  char vitomem[20];
//   opentty("/dev/ttyUSB0");
//   vito_init();
//   sleep(30);
//   vito_close();
frame_debug = 1;
vito_request(0x5525, 2, vitomem);
}

   