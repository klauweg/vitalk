#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>

#include "vito_parameter.h"
#include "vito_io.h"

// Hier folgt pro Parameter eine C-Funktion:
     
/////////////////// ALLGEMEIN
char * read_deviceid( void )
{
  unsigned char content[2];
  static char valuestr[20];
  
  if ( vito_read(0x00f8, 2, content) < 0 )
    sprintf( valuestr, "NULL" );
  else
    {
      // Normalerweise sind die Parameter in Little Endian
      // Byteorder, aber bei der Deviceid hat sich offenbar
      // die umgekehrte Interpretation durchgesetzt:
      sprintf( valuestr, "0x%4x", (content[0] << 8) + content[1] );
    }
  
  return valuestr;
}

//////////////////// KESSEL
char * read_K_abgas_temp( void )
{
  unsigned char content[2];
  static char valuestr[20];
  float value;
  
  if ( vito_read(0x0808, 2, content) < 0 ) // Speicherbereich einlesen
    sprintf( valuestr, "NULL" ); // Im Fehlerfall NULL ausgeben (mysqlkompatibel)
  else
    {
      value = (content[1] << 8) + content[0]; // bytorder berücksichtigen
      value = value / 10;                     // Präzision konvertieren
      sprintf( valuestr, "%3.2f", value );    // Ergebnissstring bauen
    }
  return valuestr;
}

char * read_K_ist_temp( void )
{
  unsigned char content[2];
  static char valuestr[20];
  float value;
  
  if ( vito_read(0x0802, 2, content) < 0 )
    sprintf( valuestr, "NULL" );
  else
    {
      value = (content[1] << 8) + content[0];
      value = value / 10;
      sprintf( valuestr, "%3.2f", value );
    }
  return valuestr;
}

char * read_K_istTP_temp( void )
{
  unsigned char content[2];
  static char valuestr[20];
  float value;
  
  if ( vito_read(0x0810, 2, content) < 0 )
    sprintf( valuestr, "NULL" );
  else
    {
      value = (content[1] << 8) + content[0];
      value = value / 10;
      sprintf( valuestr, "%3.2f", value );
    }
  return valuestr;
}

char * read_K_soll_temp( void )
{
  unsigned char content[2];
  static char valuestr[20];
  float value;
  
  if ( vito_read(0x555a, 2, content) < 0 )
    sprintf( valuestr, "NULL" );
  else
    {
      value = (content[1] << 8) + content[0];
      value = value / 10;
      sprintf( valuestr, "%3.2f", value );
    }
  return valuestr;
}

//////////////////// WARMWASSER
char * read_WW_soll_temp( void)
{
  unsigned char content[1];
  static char valuestr[20];
  
  if ( vito_read(0x6300, 1, content) < 0 )
    sprintf( valuestr, "NULL" );
  else
    sprintf( valuestr, "%u", content[0] );

  return valuestr;
}

int write_WW_soll_temp( int temp )
{
  unsigned char content[1];
  
  if ( temp < 5 || temp > 60 )
    {
      fprintf( stderr, "WW_soll_temp: range exceeded!\n");
      return -1;
    }
  
  content[0] = temp & 0xff; // unnötig, aber deutlicher
  return vito_write(0x6300, 1, content);
}

char * read_WW_offset( void )
{
  unsigned char content[1];
  static char valuestr[20];

  if ( vito_read(0x6760, 1, content) < 0 )
    sprintf( valuestr, "NULL" );
  else
    sprintf( valuestr, "%u", content[0] );

  return valuestr;
}

char * read_WW_istTP_temp( void )
{
  unsigned char content[2];
  static char valuestr[20];
  float value;
  
  if ( vito_read(0x0812, 2, content) < 0 )
    sprintf( valuestr, "NULL" );
  else
    {
      value = (content[1] << 8) + content[0];
      value = value / 10;
      sprintf( valuestr, "%3.2f", value );
    }
  return valuestr;
}

char * read_WW_ist_temp( void )
{
  unsigned char content[2];
  static char valuestr[20];
  float value;
  
  if ( vito_read(0x0804, 2, content) < 0 )
    sprintf( valuestr, "NULL" );
  else
    {
      value = (content[1] << 8) + content[0];
      value = value / 10;
      sprintf( valuestr, "%3.2f", value );
    }
  return valuestr;
}


/////////////////// AUSSENTEMPERATUR
char * read_outdoor_TP_temp( void )
{
  unsigned char content[2];
  static char valuestr[20];
  float value;
  
  if ( vito_read(0x5525, 2, content) < 0 )
    sprintf( valuestr, "NULL" );
  else
    {
      value = (content[1] << 8) + content[0];
      value = value / 10;
      sprintf( valuestr, "%3.2f", value );
    }
  return valuestr;
}

char * read_outdoor_smooth_temp( void )
{
  unsigned char content[2];
  static char valuestr[20];
  float value;
  
  if ( vito_read(0x5527, 2, content) < 0 )
    sprintf( valuestr, "NULL" );
  else
    {
      value = (content[1] << 8) + content[0];
      value = value / 10;
      sprintf( valuestr, "%3.2f", value );
    }
  return valuestr;
}

char * read_outdoor_temp( void )
{
  unsigned char content[2];
  static char valuestr[20];
  float value;
  
  if ( vito_read(0x0800, 2, content) < 0 )
    sprintf( valuestr, "NULL" );
  else
    {
      value = (content[1] << 8) + content[0];
      value = value / 10;
      sprintf( valuestr, "%3.2f", value );
    }
  return valuestr;
}

/////////////////// BRENNER
char * read_starts( void )
{
  unsigned char content[4];
  static char valuestr[20];
  unsigned int value;
  
  if ( vito_read(0x088A, 4, content) < 0 )
    sprintf( valuestr, "NULL" );
  else
    {
      value = content[0] + (content[1] << 8) + (content[2] << 16) + (content[3] << 24);
      sprintf( valuestr, "%u", value );
    }
  return valuestr;
}

char * read_runtime( void )
{
  unsigned char content[4];
  static char valuestr[20];
  unsigned int value;
  
  if ( vito_read(0x0886, 4, content) < 0 )
    sprintf( valuestr, "NULL" );
  else
    {
      value = content[0] + (content[1] << 8) + (content[2] << 16) + (content[3] << 24);
      sprintf( valuestr, "%u", value );
    }
  return valuestr;
}

char * read_power( void )
{
  unsigned char content[1];
  static char valuestr[20];
  float value;
  
  if ( vito_read(0xa38f, 1, content) < 0 )
    sprintf( valuestr, "NULL" );
  else
    {
      value = content[0] / 2.0;
      sprintf( valuestr, "%3.1f", value );
    }
  return valuestr;
}



#if 0

getStatusFlamme 55d3 B_STF 1
getVerriegelung 55d3 B_STV 1

/////////////////// VENTILE UND RELAIS
getStatusUmschaltVentil 0a10 E_STUV2 1


getDrehzahlSollPInt 0a3c pc 1
getStatusPIntern 7660 E_ST 1
getDrehzahlPIntern 7660 PC 1 ???
getTempSollVL_A1M1 2544 td 2

 
#endif