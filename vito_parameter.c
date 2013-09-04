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

// Wird global als Speicher für das Ergebnis der Lesefunktionen
// als Text verwendet. Zugegriffen wird aber nur über die per Rückgabewert
// der Lesefunktionen übergebene Adresse:
char valuestr[40];

// Wird global definiert, aber nur lokal in jeder Funktion zur übergabe des Speicherinhalts
// aus bzw. an die Vitodens verwendet. Semantisch wohl nicht ganz sauber, aber wie könnte
// man sonst vermeiden das Array in jeder funktion neu zu definieren?
static uint8_t content[30];

// Hier folgt pro Parameter eine C-Funktion:
// Rückgabewert bei den Lesefunktionen ist ein Pointer
// auf ein char-array mit dem Ergebnistext.
/////////////////// ALLGEMEIN
char * read_deviceid( void )
{
  if ( vito_read(0x00f8, 2, content) < 0 ) sprintf( valuestr, "NULL" );
  else
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
  if ( vito_read(0x2323, 1, content) < 0 ) sprintf( valuestr, "NULL" );
  else
    sprintf( valuestr, "%u", content[0] );
  return valuestr;
}

int write_mode_numeric( int mode )
{
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
  if ( vito_read(0x2323, 1, content) < 0 ) sprintf( valuestr, "NULL" );
  else
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
  float value;
  
  if ( vito_read(0x0808, 2, content) < 0 ) sprintf( valuestr, "NULL" );
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
  float value;
  
  if ( vito_read(0x0802, 2, content) < 0 ) sprintf( valuestr, "NULL" );
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
  float value;
  
  if ( vito_read(0x0810, 2, content) < 0 ) sprintf( valuestr, "NULL" );
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
  float value;
  
  if ( vito_read(0x555a, 2, content) < 0 ) sprintf( valuestr, "NULL" );
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
  if ( vito_read(0x6300, 1, content) < 0 ) sprintf( valuestr, "NULL" );
  else
    sprintf( valuestr, "%u", content[0] );

  return valuestr;
}

int write_WW_soll_temp( int temp )
{
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
  static char valuestr[20];

  if ( vito_read(0x6760, 1, content) < 0 ) sprintf( valuestr, "NULL" );
  else
    sprintf( valuestr, "%u", content[0] );

  return valuestr;
}

char * read_WW_istTP_temp( void )
{
  float value;
  
  if ( vito_read(0x0812, 2, content) < 0 ) sprintf( valuestr, "NULL" );
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
  float value;
  
  if ( vito_read(0x0804, 2, content) < 0 ) sprintf( valuestr, "NULL" );
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
  float value;
  
  if ( vito_read(0x5525, 2, content) < 0 ) sprintf( valuestr, "NULL" );
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
  float value;
  
  if ( vito_read(0x5527, 2, content) < 0 ) sprintf( valuestr, "NULL" );
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
  float value;
  
  if ( vito_read(0x0800, 2, content) < 0 ) sprintf( valuestr, "NULL" );
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
  unsigned int value;
  
  if ( vito_read(0x088A, 4, content) < 0 ) sprintf( valuestr, "NULL" );
  else
    {
      value = content[0] + (content[1] << 8) + (content[2] << 16) + (content[3] << 24);
      sprintf( valuestr, "%u", value );
    }
  return valuestr;
}

char * read_runtime( void )
{
  unsigned int value;
  
  if ( vito_read(0x0886, 4, content) < 0 ) sprintf( valuestr, "NULL" );
  else
    {
      value = content[0] + (content[1] << 8) + (content[2] << 16) + (content[3] << 24);
      sprintf( valuestr, "%u", value );
    }
  return valuestr;
}

char * read_power( void )
{
  float value;
  
  if ( vito_read(0xa38f, 1, content) < 0 ) sprintf( valuestr, "NULL" );
  else
    {
      value = content[0] / 2.0;
      sprintf( valuestr, "%3.1f", value );
    }
  return valuestr;
}

/////////////////// HYDRAULIK
char * read_ventil_numeric( void )
{
  if ( vito_read(0x0a10, 1, content) < 0 ) sprintf( valuestr, "NULL" );
  else
    sprintf( valuestr, "%u", content[0] );

  return valuestr;
}

char * read_ventil( void )
{
  if ( vito_read(0x0a10, 1, content) < 0 ) sprintf( valuestr, "NULL" );
  else
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
  if ( vito_read(0x0a3c, 1, content) < 0 ) sprintf( valuestr, "NULL" );
  else
    sprintf( valuestr, "%u", content[0] );

  return valuestr;
}

/////////////////// HEIZKREIS
char * read_VL_soll_temp( void )
{
  float value;
  
  if ( vito_read(0x2544, 2, content) < 0 ) sprintf( valuestr, "NULL" );
  else
    {
      value = (content[1] << 8) + content[0];
      value = value / 10;
      sprintf( valuestr, "%3.2f", value );
    }
  return valuestr;
}

char * read_raum_soll_temp( void)
{
  if ( vito_read(0x2306, 1, content) < 0 ) sprintf( valuestr, "NULL" );
  else
    sprintf( valuestr, "%u", content[0] );

  return valuestr;
}

int write_raum_soll_temp( int temp )
{
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
  if ( vito_read(0x2307, 1, content) < 0 ) sprintf( valuestr, "NULL" );
  else
    sprintf( valuestr, "%u", content[0] );

  return valuestr;
}

int write_red_raum_soll_temp( int temp )
{
  if ( temp < 10 || temp > 30 )
    {
      fprintf( stderr, "Raum_soll_temp: range exceeded!\n");
      return -1;
    }
  
  content[0] = temp & 0xff; // unnötig, aber deutlicher
  return vito_write(0x2307, 1, content);
}

