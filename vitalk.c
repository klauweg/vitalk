#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include "vito_io.h"
#include "vito_parameter.h"

// Globals:
extern int frame_debug;

// Debug Ausgabe: Array als Hexadezimal:
static void print_hex( unsigned char *buffer, int len )
{
  int i;
  
  for ( i = 0; i < len; i++ )
    fprintf( stdout, " 0x%02x", buffer[i] );
//  fprintf( stdout, "\n" );
}
                     


int main()
{
  opentty("/dev/ttyUSB0");
  vito_init();

  fprintf( stderr, "\n" );
  
//  frame_debug = 1;

  printf("Device Id: %s\n", read_deviceid() );
  printf("Kessel ist Temperatur: %s °C\n", read_K_ist_temp() );
  printf("Kessel ist Temperatur Tiefpass: %s °C\n", read_K_istTP_temp() );
  printf("Kessel Soll Temperatur: %s °C\n", read_K_soll_temp() );
  printf("Kessel Abgastemperatur: %s °C\n", read_K_abgas_temp() );
  
  printf("Warmwasser Solltemperatur: %s °C\n", read_WW_soll_temp() );
  printf("Warmwasser Vorlaufoffset: %s K\n", read_WW_offset() );
  printf("Warmwasser ist Temperatur Tiefpass: %s °C\n", read_WW_istTP_temp() );
  printf("Warmwasser ist Temperatur: %s °C\n", read_WW_ist_temp() );
  
  printf("Aussentemperatur ist: %s °C\n", read_outdoor_temp() );
  printf("Aussentemperatur ist Tiefpass: %s °C\n", read_outdoor_TP_temp() );
  printf("Aussentemperatur ist gedämpft: %s °C\n", read_outdoor_smooth_temp() );

  printf("Brennerstarts: %s\n", read_starts() );
  printf("Brennerlaufzeit: %s s\n", read_runtime() );
  printf("Brennerleistung: %s %%\n", read_power() );
  
//  write_WW_soll_temp( 39 );
  
  vito_close();
  closetty();
  
  return 0;
}

   