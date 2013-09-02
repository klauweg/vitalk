#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>

#include "vito_parameter.h"
#include "vito_io.h"

//struct parameter[] = {
  
// ALLGEMEIN
void read_deviceid( char *valuestr )
{
  unsigned char content[2];
  int value;
  
  if ( vito_read(0x00f8, 2, content) < 0 )
    sprintf( valuestr, "NULL" );
  else
    {
      value = (content[1] << 8) + content[0];
      sprintf( valuestr, "0x%4x", value );
    }
}


// KESSEL

void read_abgas_temp( char *valuestr )
{
  unsigned char content[2];
  float value;
  
  if ( vito_read(0x0808, 2, content) < 0 )
    sprintf( valuestr, "NULL" );
  else
    {
      value = (content[1] << 8) + content[0];
      value = value / 10;
      sprintf( valuestr, "%3.2f", value );
    }
}


// 
void read_WW_soll_temp( char *valuestr )
{
  unsigned char content[1];
  float value;
  
  if ( vito_read(0x6300, 1, content) < 0 )
    sprintf( valuestr, "NULL" );
  else
    {
      value = content[0];
      sprintf( valuestr, "%3.2f", value );
    }
}
