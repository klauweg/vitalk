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
  int i, j;
  
  unsigned char vitomem[20];
  opentty("/dev/ttyUSB0");
  vito_init();

      fprintf( stderr, "\n" );
  
//  frame_debug = 1;
  for ( i = 0x1000; i <= 0x5fff; i += 8 )
    {
      fprintf( stdout, "0x%04x:", i);
      for ( j = 0; j <= 7; j++ )
	{
	  if ( vito_read(i+j, 1, vitomem) >= 0 )
	    print_hex( vitomem, 1 );
	  else
	    fprintf( stdout, "  -- " );
	}
      fprintf( stdout, "\n" );
    }
//  res = vitomem[0] << 8;
//  res+= vitomem[1];

// fprintf(stderr, "VITOMEM: %x\n", res);
  
  vito_close();
  closetty();
}

   