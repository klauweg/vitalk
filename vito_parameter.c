#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "vito_parameter.h"
#include "vito_io.h"
#include "fehlerliste.h"

#define CACHE_TIME 6

// Das prologue() Makro wird verwendet, um den Anfang der
// Parameterfunktionen zu bauen:
#define prologue() \
   static time_t old_time = 0; \
   uint8_t vitomem[30]; \
   time_t new_time = time(NULL) / (CACHE_TIME); \
   if ( new_time > old_time ) \
   {

// Das epilogue() Makro wird verwendet, um das Ende der
// Parameterfunktionen zu bauen:
#define epilogue() \
   old_time = new_time; \
   } \
   return cache;

// OBACHT:
// wenn in den vito_read() Aufrufen der Parameterfunktionen ein
// Fehler auftritt (-1), muss gleich mit return beendet werden,
// damit old_time=new_time nicht erreicht wird und beim naechsten
// Aufruf ein fehlerhafter Wert aus dem Cache statt einem Fehler
// zurueckgegeben wird.

////////////////////////// PARAMETERFUNKTIONEN ////////////////////

/* -------------------------------- */
const char * const read_deviceid( void )
{
  static char cache[10];
  prologue()
    if ( vito_read( 0x00f8, 2, vitomem ) < 0 )
      return "NULL";
  // Normalerweise sind die Parameter in Little Endian
  // Byteorder, aber bei der Deviceid hat sich offenbar
  // die umgekehrte Interpretation durchgesetzt:
  sprintf( cache, "0x%4x", (vitomem[0] << 8) + vitomem[1] );
  epilogue()
}

/* -------------------------------- */
const char * const read_mode( void )
{
  static char cache[5];
  prologue()
    if ( vito_read( 0x2323, 1, vitomem ) < 0 )
      return "NULL";
  sprintf( cache, "%u", vitomem[0] );
  epilogue()
}

/* -------------------------------- */
const char * const write_mode( const char * value_str )
{
  uint8_t content[3];
  int mode;
  
  mode = atoi( value_str );
  // Dauernd reduziert und dauernd normal unterstuetzt meine Vitodens offenbar nicht:
  if ( mode < 0 || mode > 2 )
    return "Illegal Mode!";

  content[0] = mode & 0xff; // unnoetig, aber deutlicher
  if ( vito_write(0x2323, 1, content) < 0 )
    return "Vitodens communication Error";
  else
    return "OK";
}

/* -------------------------------- */
const char * const read_mode_text( void )
{
  const char * const mode = read_mode();
  
  if ( strcmp(mode,"NULL") == 0)
    return "NULL";

  switch ( atoi(mode) )
	{
	case 0: return "Abschaltbetrieb";
	case 1: return "Nur Warmwasser";
	case 2: return "Heizen und Warmwasser";
	default: return "UNKNOWN";
	}
}

/* -------------------------------- */
// Das Auslesen der Fehlerliste ist etwas konfus, denn in dem
// vito.xml file von ceteris paribus werden 9 byte pro Eintrag
// gelesen. Ich sehe den Sinn aber nicht? Ich muss mal noch beobachten was passiert
// wenn mehr als ein Eintrag in der Fehlerliste steht!
const char * const read_errors( void )
{
  static char cache[50];
  prologue()
    
  int i;
  
  // Die 10 Fehlermeldungen einlesen:
  for ( i=0; i <= 9; i++)
    {
      if ( vito_read( 0x7507 + (i*0x0009), 1, vitomem ) < 0 )
	return "NULL";
      
      sprintf( cache + (4*i), "%03u,", vitomem[0] );
    }
  cache[39]='\0'; //letztes Komma ueberschreiben

  epilogue()
}

/* -------------------------------- */
// Die 10 Fehlerspeicherplaetze als Textstrings zurueckgeben:
const char * const read_errors_text( void )
{
  static char cache[80*10];
  char zeile[80];
  const char * errors;
  int n_error;
  int i;
  
  errors = read_errors();
  if ( strcmp( errors, "NULL" ) == 0 )
    return "NULL";

  cache[0] = '\0';
  for ( i = 0; i <= 9; i++ ) // 10 Fehler
    {
      n_error = atoi(errors + (i*4));
      assert ( n_error >= 0 && n_error <= 255 );
      sprintf( zeile, "0x%02x %s\n", n_error, fehlerliste[n_error] );
      strcat( cache, zeile );
    }

  return cache;
}


//////////////////// KESSEL
/* -------------------------------- */
const char * const read_abgas_temp( void )
{
  static char cache[6];
  prologue()
    if ( vito_read( 0x0808, 2, vitomem) < 0 )
      return "NULL";
  sprintf( cache, "%3.2f", ( vitomem[0] + (vitomem[1] << 8)) / 10.0 );
  epilogue()
}

/* -------------------------------- */
const char * const read_k_ist_temp( void )
{
  static char cache[6];
  prologue()
    if ( vito_read( 0x0802, 2, vitomem) < 0 )
      return "NULL";
  sprintf( cache, "%3.2f", ( vitomem[0] + (vitomem[1] << 8)) / 10.0 );
  epilogue()
}

const char * const read_k_ist_temp_tp( void )
{
  static char cache[6];
  prologue()
    if ( vito_read( 0x0810, 2, vitomem) < 0 )
      return "NULL";
  sprintf( cache, "%3.2f", ( vitomem[0] + (vitomem[1] << 8)) / 10.0 );
  epilogue()
}

const char * const read_k_soll_temp( void )
{
  static char cache[6];
  prologue()
    if ( vito_read( 0x555a, 2, vitomem) < 0 )
      return "NULL";
  sprintf( cache, "%3.2f", ( vitomem[0] + (vitomem[1] << 8)) / 10.0 );
  epilogue()
}

//////////////////// WARMWASSER
/* -------------------------------- */
const char * const read_ww_soll_temp( void)
{
  static char cache[6];
  prologue()
    if ( vito_read( 0x6300, 1, vitomem) < 0 )
      return "NULL";
  sprintf( cache, "%u", vitomem[0] );
  epilogue()
}

/* -------------------------------- */
const char * const write_ww_soll_temp( const char *value_str )
{
  uint8_t content[3];
  int temp;
  
  temp = atoi( value_str );
  if ( temp < 5 || temp > 60 )
    return "WW_soll_temp: range exceeded!";

  content[0] = temp & 0xff; // unnoetig, aber deutlicher
  if ( vito_write(0x6300, 1, content) < 0 )
    return "Vitodens communication Error";
  else
    return "OK";
}

/* -------------------------------- */
const char * const read_ww_offset( void)
{
  static char cache[6];
  prologue()
    if ( vito_read( 0x6760, 1, vitomem) < 0 )
      return "NULL";
  sprintf( cache, "%u", vitomem[0] );
  epilogue()
}

/* -------------------------------- */
const char * const read_ww_ist_temp_tp( void )
{
  static char cache[6];
  prologue()
    if ( vito_read( 0x0812, 2, vitomem) < 0 )
      return "NULL";
  sprintf( cache, "%3.2f", ( vitomem[0] + (vitomem[1] << 8)) / 10.0 );
  epilogue()
}

/* -------------------------------- */
const char * const read_ww_ist_temp( void )
{
  static char cache[6];
  prologue()
    if ( vito_read( 0x0804, 2, vitomem) < 0 )
      return "NULL";
  sprintf( cache, "%3.2f", ( vitomem[0] + (vitomem[1] << 8)) / 10.0 );
  epilogue()
}


/////////////////// AUSSENTEMPERATUR
/* -------------------------------- */
const char * const read_outdoor_temp_tp( void )
{
  static char cache[6];
  prologue()
    if ( vito_read( 0x5525, 2, vitomem) < 0 )
      return "NULL";
  sprintf( cache, "%3.2f", ((int16_t)( vitomem[0] + (vitomem[1] << 8))) / 10.0 );
  epilogue()
}

/* -------------------------------- */
const char * const read_outdoor_temp_smooth( void )
{
  static char cache[6];
  prologue()
    if ( vito_read( 0x5527, 2, vitomem) < 0 )
      return "NULL";
  sprintf( cache, "%3.2f", ((int16_t)( vitomem[0] + (vitomem[1] << 8))) / 10.0 );
  epilogue()
}

/* -------------------------------- */
const char * const read_outdoor_temp( void )
{
  static char cache[6];
  prologue()
    if ( vito_read( 0x0800, 2, vitomem) < 0 )
      return "NULL";
  sprintf( cache, "%3.2f", ((int16_t)( vitomem[0] + (vitomem[1] << 8))) / 10.0 );
  epilogue()
}

/////////////////// BRENNER
/* -------------------------------- */
const char * const read_starts(void )
{
  static char cache[20];
  prologue()
    if ( vito_read( 0x088A, 4, vitomem) < 0 )
      return "NULL";
  
  unsigned int value;
  
  value = vitomem[0] + (vitomem[1] << 8) + (vitomem[2] << 16) + (vitomem[3] << 24);
      sprintf( cache, "%u", value );

  epilogue()
}

/* -------------------------------- */
const char * const read_runtime( void )
{
  static char cache[20];
  prologue()
    if ( vito_read( 0x0886, 4, vitomem) < 0 )
      return "NULL";
  
  unsigned int value;
  
  value = vitomem[0] + (vitomem[1] << 8) + (vitomem[2] << 16) + (vitomem[3] << 24);
      sprintf( cache, "%u", value );

  epilogue()
}

/* -------------------------------- */
const char * const read_runtime_h( void )
{
  static char cache[20];
  const char *result;

  result = read_runtime();
  
  if ( strcmp( result, "NULL" ) == 0 )
    return "NULL";

  sprintf( cache, "%06.1f", atoi(result) / 3600.0 );
  return cache;
}

/* -------------------------------- */
const char * const read_power( void )
{
  static char cache[6];
  prologue()
    if ( vito_read( 0xa38f, 1, vitomem) < 0 )
      return "NULL";
  
  sprintf( cache, "%3.1f", vitomem[0] / 2.0 );
  epilogue()
}

/////////////////// HYDRAULIK
/* -------------------------------- */
const char * const read_ventil( void )
{
  static char cache[5];
  prologue()
    if ( vito_read( 0x0a10, 1, vitomem) < 0 )
      return "NULL";
  
  sprintf( cache, "%u", vitomem[0] );
  epilogue()
}

/* -------------------------------- */
const char * const read_ventil_text( void )
{
  const char *result;
  
  result = read_ventil();
  
  if ( strcmp( result, "NULL" ) == 0 )
    return "NULL";
  
  switch (atoi(result))
    {
    case 0: return "undefiniert";
    case 1: return "Heizkreis";
    case 2: return "Mittelstellung";
    case 3: return "Warmwasserbereitung";
    default: return "UNKNOWN";
    }
}

/* -------------------------------- */
const char * const read_pump_power( void )
{
  static char cache[5];
  prologue()
    if ( vito_read( 0x0a3c, 1, vitomem) < 0 )
      return "NULL";

  sprintf( cache, "%u", vitomem[0] );
  epilogue()
}

/* -------------------------------- */
const char * const read_flow( void )
{
  static char cache[10];
  prologue()
     if ( vito_read( 0x0c24, 2, vitomem) < 0 )
      return "NULL";
  
  sprintf( cache, "%u", (vitomem[0] + (vitomem[1] << 8)) );
  epilogue()
}
    
/////////////////// HEIZKREIS
/* -------------------------------- */
const char * const read_vl_soll_temp( void )
{
  static char cache[6];
  prologue()
    if ( vito_read( 0x2544, 2, vitomem) < 0 )
      return "NULL";
  sprintf( cache, "%3.2f", ( vitomem[0] + (vitomem[1] << 8)) / 10.0 );
  epilogue()
}

/* -------------------------------- */
const char * const read_raum_soll_temp( void )
{
  static char cache[6];
  prologue()
    if ( vito_read( 0x2306, 1, vitomem) < 0 )
      return "NULL";
  sprintf( cache, "%u", vitomem[0] );
  epilogue()
}

/* -------------------------------- */
const char * const write_raum_soll_temp( const char *value_str )
{
  uint8_t content[3];
  int temp;
  
  temp = atoi( value_str );
  
  if ( temp < 10 || temp > 30 )
    return "Raum_soll_temp: range exceeded!";
  
  content[0] = temp & 0xff; // unnoetig, aber deutlicher
  if ( vito_write(0x2306, 1, content) < 0 )
    return "Vitodens communication Error";
  else
    return "OK";
}

/* -------------------------------- */
const char * const read_red_raum_soll_temp( void )
{
  static char cache[6];
  prologue()
    if ( vito_read( 0x2307, 1, vitomem) < 0 )
      return "NULL";
  sprintf( cache, "%u", vitomem[0] );
  epilogue()
}

/* -------------------------------- */
const char * const write_red_raum_soll_temp( const char *value_str )
{
  uint8_t content[3];
  int temp;
  
  temp = atoi( value_str );
  
  if ( temp < 10 || temp > 30 )
    return "Raum_soll_temp: range exceeded!";
  
  content[0] = temp & 0xff; // unnoetig, aber deutlicher
  if ( vito_write(0x2307, 1, content) < 0 )
    return "Vitodens communication Error";
  else
    return "OK";
}

/* -------------------------------- */
const char * const read_neigung( void )
{
  static char cache[6];
  prologue()
    if ( vito_read( 0x27d3, 1, vitomem) < 0 )
      return "NULL";
  sprintf( cache, "%2.1f", vitomem[0] / 10.0 );
  epilogue()
}

/* -------------------------------- */
const char * const read_niveau( void )
{
  static char cache[5];
  prologue()
    if ( vito_read( 0x27d4, 1, vitomem) < 0 )
      return "NULL";
  sprintf( cache, "%u", vitomem[0] );
  epilogue()
}

//////////////////////////////////////////////////////////////////////////
// obacht: maximale Befehlslaenge 20 Zeichen, sonst klemmt der telnet-parser
const struct s_parameter parameter_liste[] = {
  { "errors", "Error History (numerisch)", "", P_ERRORS, &read_errors, NULL },
  { "errors_text", "Error History (text)", "", P_ERRORS, &read_errors_text, NULL },
  { "deviceid", "Geraeteidentifikation", "", P_ALLGEMEIN, &read_deviceid, NULL },
  { "mode", "Betriebsmodus (numerisch)", "", P_ALLGEMEIN, &read_mode, &write_mode },
  { "mode_text", "Betriebsmodus (text)", "", P_ALLGEMEIN, &read_mode_text, NULL },
  { "outdoor_temp", "Aussentemperatur", "oC", P_ALLGEMEIN, &read_outdoor_temp, NULL },
  { "outdoor_temp_tp", "Aussentemperatur Tiefpass", "oC", P_ALLGEMEIN, &read_outdoor_temp_tp, NULL },
  { "outdoor_temp_smooth", "Aussentemperatur Gedaempft", "oC", P_ALLGEMEIN, &read_outdoor_temp_smooth, NULL },
  { "k_ist_temp", "Kessel Ist Temperatur", "oC", P_KESSEL, &read_k_ist_temp, NULL },
  { "k_ist_temp_tp", "Kessel Ist T. nach Tiefpass", "oC", P_KESSEL, &read_k_ist_temp_tp, NULL },
  { "k_soll_temp", "Kessel Soll Temperatur", "oC", P_KESSEL, &read_k_soll_temp, NULL },
  { "k_abgas_temp", "Kessel Abgastemperatur", "oC", P_KESSEL, &read_abgas_temp, NULL },
  { "ww_soll_temp", "Warmwasser Soll Temperatur", "oC", P_WARMWASSER, &read_ww_soll_temp, &write_ww_soll_temp },
  { "ww_ist_temp", "Warmwasser Ist Temperatur", "oC", P_WARMWASSER, &read_ww_ist_temp, NULL },
  { "ww_ist_temp_tp", "Warmwasser Ist Temp. Tiefpass", "oC", P_WARMWASSER, &read_ww_ist_temp_tp, NULL },
  { "ww_offset", "Offset Kessel/WW Soll", "K", P_WARMWASSER, &read_ww_offset, NULL },
  { "vl_soll_temp", "Vorlauf Solltemperatur", "oC", P_HEIZKREIS, &read_vl_soll_temp, NULL },
  { "raum_soll_temp", "Raum Solltemperatur", "oC", P_HEIZKREIS, &read_raum_soll_temp, &write_raum_soll_temp },
  { "red_raum_soll_temp", "Reduzierte Raum Solltemperatur", "oC", P_HEIZKREIS, &read_red_raum_soll_temp, &write_red_raum_soll_temp },
  { "niveau", "Heizkurve Niveau", "K", P_HEIZKREIS, &read_niveau, NULL },
  { "neigung", "Heizkurve Neigung", "", P_HEIZKREIS, &read_neigung, NULL },
  { "starts", "Brennerstarts", "", P_BRENNER, &read_starts, NULL },
  { "runtime_h", "Brennerlaufzeit", "h", P_BRENNER, &read_runtime_h, NULL },
  { "runtime", "Brennerlaufzeit", "s", P_BRENNER, &read_runtime, NULL },
  { "power", "Brennerleistung", "%", P_BRENNER, &read_power, NULL },
  { "ventil", "Ventilstellung", "", P_HYDRAULIK, &read_ventil, NULL },
  { "ventil_text", "Ventilstellung", "", P_HYDRAULIK, &read_ventil_text, NULL },
  { "pump_power", "Pumpenleistung", "%", P_HYDRAULIK, &read_pump_power, NULL },
  { "flow", "Volumenstrom", "l/h", P_HYDRAULIK, &read_flow, NULL },
  { NULL, NULL, NULL, 0, NULL, NULL }
};

/////////////////////////////////////////////////////////////////////////////////

// Parameter Wert nachschlagen nach Name
const char * const get_v( const char *name )
{
  int i=0;
  
  while( parameter_liste[i].p_name ) // Ende der Liste ?
    {
      if ( strcmp( name, parameter_liste[i].p_name ) == 0 ) // Parametername gefunden?
	{
	  if ( parameter_liste[i].f_read ) // Gibts eine Zugriffsfunktion?
	    return parameter_liste[i].f_read();
	  else
	    return "Function not implemented.";
	}
      i++;
    }

  return "Parameter not found.";
}

// Einheit fuer Parameter nachschlagen
const char * const get_u( const char *name )
{
  int i=0;
  
  while( parameter_liste[i].p_name ) // Ende der Liste
    {
      if ( strcmp( name, parameter_liste[i].p_name ) == 0 ) // Parametername gefunden?
	return parameter_liste[i].p_einheit;

      i++;
    }

  return "Parameter not found.";
}

// Parameterwert setzen
const char * const set_v( const char *name, const char *value )
{
  int i=0;
  
  while( parameter_liste[i].p_name ) // Ende der Liste ?
    {
      if ( strcmp( name, parameter_liste[i].p_name ) == 0 ) // Parametername gefunden?
	{
	  if ( parameter_liste[i].f_write ) // Gibts eine Schreibfunktion?
	      return parameter_liste[i].f_write( value );
	  else
	    return "Read only!";
	}
      i++;
    }

  return "Unknown Parameter!";
}

