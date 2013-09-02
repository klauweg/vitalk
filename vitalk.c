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

// Debug Ausgabe: Array als Hexadezimal:
static int print_hex( unsigned char *buffer, int len )
{
  int i;
  
  for ( i = 0; i < len; i++ )
    fprintf( stdout, " 0x%02x", buffer[i] );
//  fprintf( stdout, "\n" );
}
                     


main()
{
  char buffer[20];
  opentty("/dev/ttyUSB0");
  vito_init();

  fprintf( stderr, "\n" );
  
  frame_debug = 1;

  read_WW_soll_temp( buffer );
  fprintf( stdout, "Warmwasser: %s\n", buffer );
  read_abgas_temp( buffer );
  fprintf( stdout, "Abgas: %s\n", buffer );
  read_deviceid( buffer );
  fprintf( stdout, "Device: %s\n", buffer );

//  res = vitomem[0] << 8;
//  res+= vitomem[1];

// fprintf(stderr, "VITOMEM: %x\n", res);

  
  vito_close();
  closetty();
}

   