#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "vito_io.h"
#include "vito_parameter.h"
//#include "flowcandidates.h"
#include "fullscan.h"

// Globals:
extern int frame_debug;

// Signal Handler:
void exit_handler( int exitcode )
{
  fprintf(stderr, "Abort caught, closing I/O Channels....\n" );
  sleep(5);
  vito_close();
  closetty();
  exit( exitcode );
}

int main()
{
  signal(SIGINT, exit_handler);
  signal(SIGHUP, exit_handler);
  
  opentty("/dev/ttyUSB0");
  vito_init();

write_mode_numeric( 2 );

    int i;
  uint8_t newval[2];
  uint16_t value;
  
  for(;;)
    {
      vito_read( 0x0c24, 2, newval );
      value = (newval[1]<<8) + newval[0];
      printf("0x0c24 : 0x%02x 0x%02x, %u\n", newval[0], newval[1], value);
      sleep (1);
    }
  
//write_red_raum_soll_temp( 15 );
  
//goto end;
  
  
  printf("\033[2J\033[;H");
  
//  for (;;)
//    {
      printf("\033[H");
  for (i=0; i< sizeof(fullscan)/4; i++ )
    {
      vito_read( fullscan[i][0], 2, newval );
      printf("0x%04x : 0x%02x 0x%02x\n", fullscan[i][0], newval[0], newval[1]);
    }

//sleep(1);
//    }

  vito_close();
  closetty();
  
  return 0;
}

