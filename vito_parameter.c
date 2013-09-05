#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>

#include "vito_parameter.h"
#include "vito_io.h"

// Das prologue() Makro wird verwendet um den Anfang der
// Parameterfunktionen zu bauen:
#define prologue( address, length ) \
   static char valuestr[40]; \
   uint8_t content[30]; \
   if ( vito_read( (address), (length), content ) < 0 ) \
     sprintf( valuestr, "NULL" ); \
   else

// Funktion zum interpretieren von 16bit Daten im content-array als
// (signed) int16_t mit LSB first, MSB last (Viessmann Data Byteorder)
// Gültig für übliche 2-byte Temperaturparameter mit 0,1°C Auflösung
static void interpret_int16_t( uint8_t *content, char *valuestr )
{
  int16_t value;
  
  value = content[0];
  value += ( content[1] << 8 );

  sprintf( valuestr, "%3.2f", value / 10.0 );
}

////////////////////////// PARAMETERFUNKTIONEN ////////////////////

// Hier folgt pro Parameter eine C-Funktion:
// Rückgabewert bei den Lesefunktionen ist ein Pointer
// auf ein char-array mit dem Ergebnistext.
/////////////////// ALLGEMEIN
char * read_deviceid( void )
{
  prologue( 0x00f8, 2 )
    {
      // Normalerweise sind die Parameter in Little Endian
      // Byteorder, aber bei der Deviceid hat sich offenbar
      // die umgekehrte Interpretation durchgesetzt:
      sprintf( valuestr, "0x%4x", (content[0] << 8) + content[1] );
    }
  return valuestr;
}

char * read_mode_numeric( void )
{
  prologue( 0x2323, 1 )
    {
      sprintf( valuestr, "%u", content[0] );
    }
  return valuestr;
}

int write_mode_numeric( int mode )
{
  uint8_t content[30];
  // Dauernd reduziert und dauernd normal unterstützt meine Vitodens offenbar nicht
  if ( mode < 0 || mode > 2 )
    {
      fprintf( stderr, "Illegal Mode!\n");
      return -1;
    }
  
  content[0] = mode & 0xff; // unnötig, aber deutlicher
  return vito_write(0x2323, 1, content);
}

char * read_mode( void )
{
  prologue( 0x2323, 1 )
    {
      switch (content[0])
	{
	case 0: strcpy( valuestr, "Abschaltbetrieb" );
	  break;
	case 1: strcpy( valuestr, "Nur Warmwasser" );
	  break;
	case 2: strcpy( valuestr, "Heizen und Warmwasser" );
	  break;
	default: sprintf( valuestr, "UNKNOWN: %u", content[0] );
	  break;
	}
    }
  return valuestr;
}

//////////////////// KESSEL
char * read_K_abgas_temp( void )
{
  prologue( 0x0808, 2 )
    interpret_int16_t( content, valuestr );
  return valuestr;
}

char * read_K_ist_temp( void )
{
  prologue( 0x0802, 2 )
    interpret_int16_t( content, valuestr );
  return valuestr;
}

char * read_K_istTP_temp( void )
{
  prologue( 0x0810, 2 )
    interpret_int16_t( content, valuestr );
  return valuestr;
}

char * read_K_soll_temp( void )
{
  prologue( 0x555a, 2 )
    interpret_int16_t( content, valuestr );
  return valuestr;
}

//////////////////// WARMWASSER
char * read_WW_soll_temp( void)
{
  prologue( 0x6300, 1 )
    sprintf( valuestr, "%u", content[0] );
  return valuestr;
}

int write_WW_soll_temp( int temp )
{
  uint8_t content[30];
  
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
  prologue( 0x6760, 1 )
    sprintf( valuestr, "%u", content[0] );
  return valuestr;
}

char * read_WW_istTP_temp( void )
{
  prologue( 0x0812, 2 )
    interpret_int16_t( content, valuestr );
  return valuestr;
}

char * read_WW_ist_temp( void )
{
  prologue( 0x0804, 2 )
    interpret_int16_t( content, valuestr );
  return valuestr;
}


/////////////////// AUSSENTEMPERATUR
char * read_outdoor_TP_temp( void )
{
  prologue( 0x5525, 2 )
    interpret_int16_t( content, valuestr );
  return valuestr;
}

char * read_outdoor_smooth_temp( void )
{
  prologue( 0x5527, 2 )
    interpret_int16_t( content, valuestr );
  return valuestr;
}

char * read_outdoor_temp( void )
{
  prologue( 0x0800, 2 )
    interpret_int16_t( content, valuestr );
  return valuestr;
}

/////////////////// BRENNER
char * read_starts( void )
{
  prologue( 0x088A, 4 )
    {
      unsigned int value;
  
      value = content[0] + (content[1] << 8) + (content[2] << 16) + (content[3] << 24);
      sprintf( valuestr, "%u", value );
    }
  return valuestr;
}

char * read_runtime( void )
{
  prologue( 0x0886, 4 )
    {
      unsigned int value;
  
      value = content[0] + (content[1] << 8) + (content[2] << 16) + (content[3] << 24);
      sprintf( valuestr, "%u", value );
    }
  return valuestr;
}

char * read_power( void )
{
  prologue( 0xa38f, 1 )
    sprintf( valuestr, "%3.1f", content[0] / 2.0 );
  return valuestr;
}

/////////////////// HYDRAULIK
char * read_ventil_numeric( void )
{
  prologue( 0x0a10, 1 )
    sprintf( valuestr, "%u", content[0] );
  return valuestr;
}

char * read_ventil( void )
{
  prologue( 0x0a10, 1 )
    {
      switch (content[0])
	{
	case 0: strcpy( valuestr, "undefiniert" );
	  break;
	case 1: strcpy( valuestr, "Heizkreis" );
	  break;
	case 2: strcpy( valuestr, "Mittelstellung" );
	  break;
	case 3: strcpy( valuestr, "Warmwasserbereitung" );
	  break;
	default: sprintf( valuestr, "UNKNOWN: %u", content[0] );
	  break;
	}
    }
  return valuestr;
}

char * read_pump_power( void )
{
  prologue( 0x0a3c, 1 )
    sprintf( valuestr, "%u", content[0] );
  return valuestr;
}

/////////////////// HEIZKREIS
char * read_VL_soll_temp( void )
{
  prologue( 0x2544, 2 )
    interpret_int16_t( content, valuestr );
  return valuestr;
}

char * read_raum_soll_temp( void)
{
  prologue( 0x2306, 1 )
    sprintf( valuestr, "%u", content[0] );
  return valuestr;
}

int write_raum_soll_temp( int temp )
{
  uint8_t content[30];
  
  if ( temp < 10 || temp > 30 )
    {
      fprintf( stderr, "Raum_soll_temp: range exceeded!\n");
      return -1;
    }
  
  content[0] = temp & 0xff; // unnötig, aber deutlicher
  return vito_write(0x2306, 1, content);
}

char * read_red_raum_soll_temp( void)
{
  prologue( 0x2307, 1 )
    sprintf( valuestr, "%u", content[0] );
  return valuestr;
}

int write_red_raum_soll_temp( int temp )
{
  uint8_t content[30];
  
  if ( temp < 10 || temp > 30 )
    {
      fprintf( stderr, "Raum_soll_temp: range exceeded!\n");
      return -1;
    }
  
  content[0] = temp & 0xff; // unnötig, aber deutlicher
  return vito_write(0x2307, 1, content);
}

