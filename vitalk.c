#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "vito_parameter.h"
#include "vito_io.h"

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

  fprintf( stderr, "\n" );
  
  
//  frame_debug = 1;
  printf("\033[2J\033[;H");

for(;;)
    {
  printf("\033[H");
  
  printf("ALLGEMEIN:\n");
  printf("Device Id: %s, Modus Numerisch: %s, Modus: %s\n",
	 read_deviceid(), read_mode_numeric(), read_mode() );

  printf("KESSEL:\n");
  printf("Kessel ist: %s °C, ist TP: %s °C\n", read_K_ist_temp(), read_K_istTP_temp() );
  printf("Kessel Soll Temperatur: %s °C\n", read_K_soll_temp() );
  printf("Kessel Abgastemperatur: %s °C\n", read_K_abgas_temp() );
  
  printf("WARMWASSER:\n");
  printf("Solltemperatur: %s °C\n", read_WW_soll_temp() );
  printf("Vorlaufoffset: %s K\n", read_WW_offset() );
  printf("ist: %s °C, ist Tiefpass: %s °C\n", read_WW_ist_temp(), read_WW_istTP_temp() );
  
  printf("AUSSENTEMPERATUR\n");
  printf("ist: %s °C, ist Tiefpass: %s °C, ist gedämpft: %s °C\n",
	 read_outdoor_temp(), read_outdoor_TP_temp(), read_outdoor_smooth_temp() );

  printf("BRENNER:\n");
  printf("Starts: %s, Laufzeit: %s s, Leistung: %s %%\n",
	 read_starts(), read_runtime(), read_power() );

  printf("HYDRAULIK:\n");
  printf("Ventilstellung Numerisch: %s, Stellung: %s\n",
	 read_ventil_numeric(), read_ventil() );
  printf("Pumpe: %s %%\n", read_pump_power() );

  printf("HEIZKREISTEMPERATUREN:\n");
  printf("Vorlaufsoll: %s °C, Raumsoll: %s °C, red. Raumsoll %s °C\n",
	 read_VL_soll_temp(), read_raum_soll_temp(), read_red_raum_soll_temp() );

      sleep(1);
    }

    //  write_WW_soll_temp( 39 );
//write_mode_numeric(0);

  vito_close();
  closetty();
  
  return 0;
}

