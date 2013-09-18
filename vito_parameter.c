#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "vito_parameter.h"
#include "vito_io.h"
#include "fehlerliste.h"

// Das prologue() Makro wird verwendet, um den Anfang der
// (meisten) Parameterfunktionen zu bauen:
#define prologue( address, length, cache_time, bufferlen ) \
   static time_t old_time = 0; \
   static char value_str[(bufferlen)]=""; \
   time_t new_time; \
   uint8_t content[30]; \
   new_time = time(NULL) / (cache_time); \
   if ( new_time > old_time ) \
   { \
     old_time = new_time; \
     if ( vito_read( (address), (length), content ) < 0 ) \
       return -1; \
     else \
     { \

// Das epilogue() Makro wird verwendet, um das Ende der
// (meisten) Parameterfunktionen zu bauen:
#define epilogue() \
   } } \
   *value_ptr = value_str; \
   return 0;


////////////////////////// PARAMETERFUNKTIONEN ////////////////////

/* -------------------------------- */
int read_deviceid( char **value_ptr )
{
  prologue( 0x00f8, 2, 240, 10 )
      // Normalerweise sind die Parameter in Little Endian
      // Byteorder, aber bei der Deviceid hat sich offenbar
      // die umgekehrte Interpretation durchgesetzt:
    sprintf( value_str, "0x%4x", (content[0] << 8) + content[1] );
  epilogue()
}

/* -------------------------------- */
int read_mode( char **value_ptr )
{
  prologue( 0x2323, 1, 6, 5 )
    sprintf( value_str, "%u", content[0] );
  epilogue()
}

/* -------------------------------- */
int write_mode( char *value_str )
{
  uint8_t content[10];
  int mode;
  
  mode = atoi( value_str );
  // Dauernd reduziert und dauernd normal unterstützt meine Vitodens offenbar nicht:
  if ( mode < 0 || mode > 2 )
    {
      fprintf( stderr, "Illegal Mode!\n");
      return -1;
    }
  
  content[0] = mode & 0xff; // unnötig, aber deutlicher
  return vito_write(0x2323, 1, content);
}

/* -------------------------------- */
int read_mode_text( char **value_ptr )
{
  char *value_str;
  char *mode;
  
  if ( read_mode( &mode ) < 0 )
    return -1;
  
  switch ( atoi(mode) )
	{
	case 0: value_str = "Abschaltbetrieb";
	  break;
	case 1: value_str = "Nur Warmwasser";
	  break;
	case 2: value_str = "Heizen und Warmwasser";
	  break;
	default: value_str = "UNKNOWN";
	  break;
	}
  
  *value_ptr = value_str;
  return 0;
}

/* -------------------------------- */
// Das Auslesen der Fehlerliste ist etwas konfus, denn in dem
// vito.xml file von ceteris paribus werden 9 byte pro Eintrag
// gelesen. Ich sehe den Sinn aber nicht? Ich muss mal noch beobachten was passiert
// wenn mehr als ein Eintrag in der Fehlerliste steht!
int read_errors( char **value_ptr )
{
  static time_t old_time = 0;
  static char value_str[80] = "";
  time_t new_time;
  uint8_t content[15];
  int address, i;
  
  new_time = time(NULL) / 60;
  if ( new_time > old_time )
    {
      old_time = new_time;
      
      // Die 10 Fehlermeldungen numerisch ins content Array lesen:
      i = 0;
      for ( address = 0x7507; address <= 0x7558; address += 9 )
	if ( vito_read( address, 1, &content[i++] ) < 0 )
	  return -1;
   
      // String der Form 0,0,0,0,0,0,0,0,0,0 basteln:
      sprintf( value_str,"%u,%u,%u,%u,%u,%u,%u,%u,%u,%u",
	     content[0],content[1],content[2],content[3],content[4],
	     content[5],content[6],content[7],content[8],content[9] );
    }
  
  *value_ptr = value_str;
  return 0;
}

/* -------------------------------- */
int read_errors_text( char **value_ptr )
{
  static char value_str[80*10] = "";
  char *errors;
  int error[10];
  
  if ( read_errors( &errors ) < 0 )
    return -1;

  // Das war ursprünglich mit strtok gelöst. Ist aber ganz böse, weil damit
  // der static puffer in read_errors() kaputt geht.
  sscanf( errors, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
       &error[0],&error[1],&error[2],&error[3],&error[4],&error[5],&error[6],&error[7],&error[8],&error[9] );

  // String mit 10 Zeilen der Fehlerbeschreibungen basteln:
  sprintf( value_str, "0x%02x %s\n0x%02x %s\n0x%02x %s\n0x%02x %s\n0x%02x %s\n"
	              "0x%02x %s\n0x%02x %s\n0x%02x %s\n0x%02x %s\n0x%02x %s\n",
	              error[0], fehlerliste[error[0]],
	              error[1], fehlerliste[error[1]],
	              error[2], fehlerliste[error[2]],
	              error[3], fehlerliste[error[3]],
	              error[4], fehlerliste[error[4]],
	              error[5], fehlerliste[error[5]],
	              error[6], fehlerliste[error[6]],
	              error[7], fehlerliste[error[7]],
	              error[8], fehlerliste[error[8]],
	              error[9], fehlerliste[error[9]] );
  
  *value_ptr = value_str;
  return 0;
}


//////////////////// KESSEL
/* -------------------------------- */
int read_abgas_temp( char **value_ptr )
{
  prologue( 0x0808, 2, 6, 6 )
    sprintf( value_str, "%3.2f", ( content[0] + (content[1] << 8)) / 10.0 );
  epilogue()
}

/* -------------------------------- */
int read_k_ist_temp( char **value_ptr )
{
  prologue( 0x0802, 2, 6, 6 )
    sprintf( value_str, "%3.2f", ( content[0] + (content[1] << 8)) / 10.0 );
  epilogue()
}

/* -------------------------------- */
int read_k_ist_temp_tp( char **value_ptr )
{
  prologue( 0x0810, 2, 6, 6 )
    sprintf( value_str, "%3.2f", ( content[0] + (content[1] << 8)) / 10.0 );
  epilogue()
}

/* -------------------------------- */
int read_k_soll_temp( char **value_ptr )
{
  prologue( 0x555a, 2, 6, 6 )
    sprintf( value_str, "%3.2f", ( content[0] + (content[1] << 8)) / 10.0 );
  epilogue()
}

//////////////////// WARMWASSER
/* -------------------------------- */
int read_ww_soll_temp( char **value_ptr)
{
  prologue( 0x6300, 1, 6, 6 )
    sprintf( value_str, "%u", content[0] );
  epilogue()
}

/* -------------------------------- */
int write_ww_soll_temp( char *value_str )
{
  uint8_t content[3];
  int temp;
  
  temp = atoi( value_str );
  if ( temp < 5 || temp > 60 )
    {
      fprintf( stderr, "WW_soll_temp: range exceeded!\n");
      return -1;
    }
  
  content[0] = temp & 0xff; // unnötig, aber deutlicher
  return vito_write(0x6300, 1, content);
}

/* -------------------------------- */
int read_ww_offset( char **value_ptr )
{
  prologue( 0x6760, 1, 60, 5 )
    sprintf( value_str, "%u", content[0] );
  epilogue()
}

/* -------------------------------- */
int read_ww_ist_temp_tp( char **value_ptr )
{
  prologue( 0x0812, 2, 6, 6 )
    sprintf( value_str, "%3.2f", ( content[0] + (content[1] << 8)) / 10.0 );
  epilogue()
}

/* -------------------------------- */
int read_ww_ist_temp( char **value_ptr )
{
  prologue( 0x0804, 2, 6, 6 )
    sprintf( value_str, "%3.2f", ( content[0] + (content[1] << 8)) / 10.0 );
  epilogue()
}


/////////////////// AUSSENTEMPERATUR
/* -------------------------------- */
int read_outdoor_temp_tp( char **value_ptr )
{
  prologue( 0x5525, 2, 6, 6 )
    sprintf( value_str, "%3.2f", ( content[0] + (content[1] << 8)) / 10.0 );
  epilogue()
}

/* -------------------------------- */
int read_outdoor_temp_smooth( char **value_ptr )
{
  prologue( 0x5527, 2, 6, 6 )
    sprintf( value_str, "%3.2f", ( content[0] + (content[1] << 8)) / 10.0 );
  epilogue()
}

/* -------------------------------- */
int read_outdoor_temp( char **value_ptr )
{
  prologue( 0x0800, 2, 6, 6 )
    sprintf( value_str, "%3.2f", ( content[0] + (content[1] << 8)) / 10.0 );
  epilogue()
}

/////////////////// BRENNER
/* -------------------------------- */
int read_starts( char **value_ptr )
{
  prologue( 0x088A, 4, 6, 20 )
    {
      unsigned int value;
  
      value = content[0] + (content[1] << 8) + (content[2] << 16) + (content[3] << 24);
      sprintf( value_str, "%u", value );
    }
  epilogue()
}

/* -------------------------------- */
int read_runtime( char **value_ptr )
{
  prologue( 0x0886, 4, 6, 20 )
    {
      unsigned int value;
  
      value = content[0] + (content[1] << 8) + (content[2] << 16) + (content[3] << 24);
      sprintf( value_str, "%u", value );
    }
  epilogue()
}

/* -------------------------------- */
int read_runtime_h( char **value_ptr )
{
  static char value_str[20] = "";
  char *result;

  if ( read_runtime( &result ) < 0 )
    return -1;

  sprintf( value_str, "%06.1f", atoi(result) / 3600.0 );

  *value_ptr = value_str;
  return 0;
}

/* -------------------------------- */
int read_power( char **value_ptr )
{
  prologue( 0xa38f, 1, 6, 6 )
    sprintf( value_str, "%3.1f", content[0] / 2.0 );
  epilogue()
}

/////////////////// HYDRAULIK
/* -------------------------------- */
int read_ventil( char **value_ptr )
{
  prologue( 0x0a10, 1, 6, 5 )
    sprintf( value_str, "%u", content[0] );
  epilogue()
}

/* -------------------------------- */
int read_ventil_text( char **value_ptr )
{
  char *value_str;
  char *buffer;
  
  if ( read_ventil( &buffer ) < 0 )
    return -1;
  
  switch (atoi(buffer))
    {
    case 0: value_str = "undefiniert";
      break;
    case 1: value_str = "Heizkreis";
      break;
    case 2: value_str = "Mittelstellung";
      break;
    case 3: value_str = "Warmwasserbereitung";
      break;
    default: value_str = "UNKNOWN: %s";
      break;
    }
  
  *value_ptr = value_str;
  return 0;
}

/* -------------------------------- */
int read_pump_power( char **value_ptr )
{
  prologue( 0x0a3c, 1, 6, 5 )
    sprintf( value_str, "%u", content[0] );
  epilogue()
}

/* -------------------------------- */
int read_flow( char **value_ptr )
{
  prologue( 0x0c24, 2, 6, 20 )
    sprintf( value_str, "%u", (content[0] + (content[1] << 8)) );
  epilogue()
}
    
/////////////////// HEIZKREIS
/* -------------------------------- */
int read_vl_soll_temp( char **value_ptr )
{
  prologue( 0x2544, 2, 6, 6 )
    sprintf( value_str, "%3.2f", ( content[0] + (content[1] << 8)) / 10.0 );
  epilogue()
}

/* -------------------------------- */
int read_raum_soll_temp( char **value_ptr )
{
  prologue( 0x2306, 1, 6, 5 )
    sprintf( value_str, "%u", content[0] );
  epilogue()
}

/* -------------------------------- */
int write_raum_soll_temp( char *value_str )
{
  uint8_t content[3];
  int temp;
  
  temp = atoi( value_str );
  
  if ( temp < 10 || temp > 30 )
    {
      fprintf( stderr, "Raum_soll_temp: range exceeded!\n");
      return -1;
    }
  
  content[0] = temp & 0xff; // unnötig, aber deutlicher
  return vito_write(0x2306, 1, content);
}

/* -------------------------------- */
int read_red_raum_soll_temp( char **value_ptr )
{
  prologue( 0x2307, 1, 6, 5 )
    sprintf( value_str, "%u", content[0] );
  epilogue()
}

/* -------------------------------- */
int write_red_raum_soll_temp( char *value_str )
{
  uint8_t content[3];
  int temp;
  
  temp = atoi( value_str );
  
  if ( temp < 10 || temp > 30 )
    {
      fprintf( stderr, "Raum_soll_temp: range exceeded!\n");
      return -1;
    }
  
  content[0] = temp & 0xff; // unnötig, aber deutlicher
  return vito_write(0x2307, 1, content);
}

/* -------------------------------- */
int read_neigung( char **value_ptr )
{
  prologue( 0x27d3, 1, 6, 6 )
    sprintf( value_str, "%2.1f", content[0] / 10.0 );
  epilogue()
}

/* -------------------------------- */
int read_niveau( char **value_ptr )
{
  prologue( 0x27d4, 1, 6, 5 )
    sprintf( value_str, "%u", content[0] );
  epilogue()
}


//////////////////////////////////////////////////////////////////////////
// obacht: maximale Befehlslänge 20 Zeichen, sonst klemmt der telnet-parser
const struct s_parameter parameter_liste[] = {
  { "errors", "Error History (numerisch)", "", P_ERRORS, &read_errors, NULL },
  { "errors_text", "Error History (text)", "", P_ERRORS, &read_errors_text, NULL },
  { "deviceid", "Geraeteidentifikation", "", P_ALLGEMEIN, &read_deviceid, NULL },
  { "mode", "Betriebsmodus (numerisch)", "", P_ALLGEMEIN, &read_mode, &write_mode },
  { "mode_text", "Betriebsmodus (text)", "", P_ALLGEMEIN, &read_mode_text, NULL },
  { "outdoor_temp", "Aussentemperatur", "°C", P_ALLGEMEIN, &read_outdoor_temp, NULL },
  { "outdoor_temp_tp", "Aussentemperatur Tiefpass", "°C", P_ALLGEMEIN, &read_outdoor_temp_tp, NULL },
  { "outdoor_temp_smooth", "Aussentemperatur Gedämpft", "°C", P_ALLGEMEIN, &read_outdoor_temp_smooth, NULL },
  { "k_ist_temp", "Kessel Ist Temperatur", "°C", P_KESSEL, &read_k_ist_temp, NULL },
  { "k_ist_temp_tp", "Kessel Ist T. nach Tiefpass", "°C", P_KESSEL, &read_k_ist_temp_tp, NULL },
  { "k_soll_temp", "Kessel Soll Temperatur", "°C", P_KESSEL, &read_k_soll_temp, NULL },
  { "k_abgas_temp", "Kessel Abgastemperatur", "°C", P_KESSEL, &read_abgas_temp, NULL },
  { "ww_soll_temp", "Warmwasser Soll Temperatur", "°C", P_WARMWASSER, &read_ww_soll_temp, &write_ww_soll_temp },
  { "ww_ist_temp", "Warmwasser Ist Temperatur", "°C", P_WARMWASSER, &read_ww_ist_temp, NULL },
  { "ww_ist_temp_tp", "Warmwasser Ist Temp. Tiefpass", "°C", P_WARMWASSER, &read_ww_ist_temp_tp, NULL },
  { "ww_offset", "Offset Kessel/WW Soll", "K", P_WARMWASSER, &read_ww_offset, NULL },
  { "starts", "Brennerstarts", "", P_BRENNER, &read_starts, NULL },
  { "runtime_h", "Brennerlaufzeit", "h", P_BRENNER, &read_runtime_h, NULL },
  { "runtime", "Brennerlaufzeit", "s", P_BRENNER, &read_runtime, NULL },
  { "power", "Brennerleistung", "%", P_BRENNER, &read_power, NULL },
  { "ventil", "Ventilstellung", "", P_HYDRAULIK, &read_ventil, NULL },
  { "ventil_text", "Ventilstellung", "", P_HYDRAULIK, &read_ventil_text, NULL },
  { "pump_power", "Pumpenleistung", "%", P_HYDRAULIK, &read_pump_power, NULL },
  { "flow", "Volumenstrom", "l/h", P_HYDRAULIK, &read_flow, NULL },
  { "vl_soll_temp", "Vorlauf Solltemperatur", "°C", P_HEIZKREIS, &read_vl_soll_temp, NULL },
  { "raum_soll_temp", "Raum Solltemperatur", "°C", P_HEIZKREIS, &read_raum_soll_temp, &write_raum_soll_temp },
  { "red_raum_soll_temp", "Reduzierte Raum Solltemperatur", "°C", P_HEIZKREIS, &read_red_raum_soll_temp, &write_red_raum_soll_temp },
  { "niveau", "Heizkurve Niveau", "K", P_HEIZKREIS, &read_niveau, NULL },
  { "neigung", "Heizkurve Neigung", "", P_HEIZKREIS, &read_neigung, NULL },
  { "", "", "", 0, NULL, NULL }
};

/////////////////////////////////////////////////////////////////////////////////

// Parameter Wert nachschlagen nach Name
char * get_v( char *name )
{
  int i=0;
  char *value_ptr = NULL;
  
  while( parameter_liste[i].p_name[0] ) // Ende der Liste ?
    {
      if ( strcmp( name, parameter_liste[i].p_name ) == 0 ) // Parametername gefunden?
	{
	  if ( parameter_liste[i].f_read ) // Gibts eine Zugriffsfunktion?
	    {
	      if ( parameter_liste[i].f_read( &value_ptr ) < 0 ) // Zugriffsfunktion aufrufen
		{
		  value_ptr="READ-ERROR"; // error calling the read function
		}
	    }
	  else
	    value_ptr="UIF-ERROR"; // Unimplemented Function Error
	  
	  return value_ptr; // Stringpointer zurückgeben
	}
      i++;
    }
  
  value_ptr="PNF-ERROR"; // Parameter not Found Error
  return value_ptr;
}

// Einheit für Parameter nachschlagen
char * get_u( char *name )
{
  int i=0;
  static char value_str[30]="";
  
  while( parameter_liste[i].p_name[0] ) // Ende der Liste
    {
      if ( strcmp( name, parameter_liste[i].p_name ) == 0 ) // Parametername gefunden?
	{
	  strcpy( value_str, parameter_liste[i].p_einheit ); // Einheit kopieren
	  return value_str; // Einheit zurückgeben
	}
      i++;
    }
  
  strcpy(value_str,"PNF-ERROR"); // Parameter not Found Error
  return value_str;
}
