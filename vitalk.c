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
#include "mysql.h"

// Signal Handler:
void exit_handler( int exitcode )
{
  fprintf(stderr, "Abort caught, closing I/O Channels....\n" );
  sleep(5);
  vito_close();
  closetty();
  exit( exitcode );
}

int main(int argc, char **argv)
{
  int c;
  char *tty_devicename = NULL;
  int my_log_intervall = 0;
  
  while ((c = getopt (argc, argv, "H:U:P:D:T:hft:c:")) != -1)
    switch(c)
      {
      case 'h':
	printf("Vitalk, Viessmann Vitodens 300 (B3HA) Interface\n"
	       " (c) by KWS, 2013\n\n"
	       "Usage: vitalk [option...]\n"
	       "  -h            give this help list\n"
	       "  -H <host>     set MySQL Hostname for Logging\n"
	       "  -U <user>     set MySQL Username for Logging\n"
	       "  -P <password> set MySQL Password for Logging\n"
	       "  -D <database> set MySQL Database Name for Logging\n"
	       "  -T <log_t>    set MySQL LogIntervall in sec.\n"
	       "  -f            activate framedebugging\n"
	       "  -t <tty_dev>  set tty Devicename to Vitodens\n"
	       "  -c <cache_t>  set parameter cache intervall in sec.\n"
               );
	exit(1);
      case 'H':
	my_hostname = optarg;
	break;
      case 'U':
	my_username = optarg;
	break;
      case 'P':
	my_password = optarg;
	break;
      case 'D':
	my_database = optarg;
	break;
      case 'T':
	sscanf( optarg, "%d", &my_log_intervall);
	break;
      case 'f':
	frame_debug = 1;
	break;
      case 't':
	tty_devicename = optarg;
	break;
      case 'c':
	sscanf( optarg, "%d", &param_cache_time );
	if ( param_cache_time < 1 ||
	     param_cache_time > 600 )
	  {
	    printf("ERROR: Cache Time must between 1 and 600 sec.!\n");
	    exit(5);
	  }
	break;
      case '?':
	exit (8);
      }
  
  if ( !tty_devicename )
    {
      printf("ERROR: Need tty Devicename!\n");
      exit(5);
    }
  
  signal(SIGINT, exit_handler);
  signal(SIGHUP, exit_handler);

  opentty( tty_devicename );
  vito_init();

  //my_query("insert into test (fieldt) values (8)")
//  exit(0);
  

//  write_WW_soll_temp( 40 );
//  write_mode_numeric(1);
//  write_raum_soll_temp( 23 );
//  write_red_raum_soll_temp( 15 );
  
  printf("\033[2J\033[;H");

for(;;)
    {
      printf("\033[H");
  
      printf("ALLGEMEIN:\n");
      printf(" Device Id: %s, Modus Numerisch: %s, Modus: %s         \n",
	     read_deviceid(), read_mode_numeric(), read_mode() );

      printf("KESSEL:\n");
      printf(" Kessel Soll Temperatur: %s °C\n", read_K_soll_temp() );
      printf(" Kessel ist: %s °C, ist TP: %s °C\n", read_K_ist_temp(), read_K_istTP_temp() );
      printf(" Kessel Abgastemperatur: %s °C\n", read_K_abgas_temp() );
  
      printf("WARMWASSER:\n");
      printf(" Solltemperatur: %s °C\n", read_WW_soll_temp() );
      printf(" Vorlaufoffset: %s K\n", read_WW_offset() );
      printf(" ist: %s °C, ist Tiefpass: %s °C\n", read_WW_ist_temp(), read_WW_istTP_temp() );
  
      printf("AUSSENTEMPERATUR\n");
      printf(" ist: %s °C, ist Tiefpass: %s °C, ist gedämpft: %s °C\n",
	     read_outdoor_temp(), read_outdoor_TP_temp(), read_outdoor_smooth_temp() );

      printf("BRENNER:\n");
      printf(" Starts: %s, Laufzeit: %s s, Laufzeit: %s h     \n",
	     read_starts(), read_runtime(), read_runtime_h() );
      printf(" Leistung: %s %%     \n", read_power() );
      
      printf("HYDRAULIK:\n");
      printf(" Ventilstellung Numerisch: %s, Stellung: %s       \n",
	     read_ventil_numeric(), read_ventil() );
      printf(" Pumpe: %s %%\n", read_pump_power() );
      printf(" Volumenstrom: %s l/h        \n", read_flow() );
      
      printf("HEIZKREISTEMPERATUREN:\n");
      printf(" Vorlaufsoll: %s °C, Raumsoll: %s °C, red. Raumsoll %s °C\n",
	     read_VL_soll_temp(), read_raum_soll_temp(), read_red_raum_soll_temp() );
      printf(" Neigung: %s,  Niveau: %s   \n", read_neigung(), read_niveau() );
      
      printf("FEHLERSPEICHER:\n");
      printf( "%s", read_error_history() );

      sleep(1);
    }


  vito_close();
  closetty();
  
  return 0;
}

