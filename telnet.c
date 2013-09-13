#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "vito_parameter.h"

#define PORT 83

static int fd_listener = 0; /* listening socket descriptor */
static int fd_telnet = 0; /* File Descriptor for Telnet */
static struct sockaddr_in serveraddr; /* server address */
static struct sockaddr_in clientaddr; /* client address */

// Per telnet aufrufbare Befehle:
const char *commands[] =
{ "help",              // 0
  "h",                 // 1
  "state",             // 2
  "s",                 // 3
  "errors",            // 4
  "e",                 // 5
  "numeric_errors",    // 6
  "set_mode",          // 7
  "set_ww",            // 8
  "set_raum",          // 9
  "set_red_raum",      // 10
  "\0"
};

// Hilfstext
static void print_help( void )
{
  dprintf(fd_telnet,
	  "Short Help Text:\n"
	  "  h, help        - This Help Text\n"
	  "  s, state       - Summary of Parameter Values\n"
	  "  e, errors      - Error History\n"
          "  numeric_errors - Error History in numeric Format\n"
	  "Changeable Parameters:\n"
	  "  set_mode       - Betriebsmodus setzen\n"
	  "        0 == Abschaltbetrieb\n"
	  "        1 == Nur Warmwasser\n"
	  "        2 == Heizen und Warmwasser\n"
	  "  set_ww         - Warmwassertemperatur in Grad Celsius setzen\n"
	  "  set_raum       - Raumtemperatur in Grad Celsius setzen\n"
	  "  set_red_raum   - reduzierte Raumtemperatur in Grad Celsius setzen\n"
	  "\n"
    );
}

// Debug Ausgabe aller Parameter
static void print_all( void )
{
  dprintf(fd_telnet, "ALLGEMEIN:\n");
  dprintf(fd_telnet, " Device Id: %s, Modus Numerisch: %s, Modus: %s         \n",
	 read_deviceid(), read_mode_numeric(), read_mode() );

  dprintf(fd_telnet, "KESSEL:\n");
  dprintf(fd_telnet, " Kessel Soll Temperatur: %s °C     \n", read_K_soll_temp() );
  dprintf(fd_telnet, " Kessel ist: %s °C, ist TP: %s °C      \n", read_K_ist_temp(), read_K_istTP_temp() );
  dprintf(fd_telnet, " Kessel Abgastemperatur: %s °C       \n", read_K_abgas_temp() );

  dprintf(fd_telnet, "WARMWASSER:\n");
  dprintf(fd_telnet, " Solltemperatur: %s °C    \n", read_WW_soll_temp() );
  dprintf(fd_telnet, " Vorlaufoffset: %s K     \n", read_WW_offset() );
  dprintf(fd_telnet, " ist: %s °C, ist Tiefpass: %s °C     \n", read_WW_ist_temp(), read_WW_istTP_temp() );
  
  dprintf(fd_telnet, "AUSSENTEMPERATUR\n");
  dprintf(fd_telnet, " ist: %s °C, ist Tiefpass: %s °C, ist gedämpft: %s °C      \n",
	 read_outdoor_temp(), read_outdoor_TP_temp(), read_outdoor_smooth_temp() );

  dprintf(fd_telnet, "BRENNER:\n");
  dprintf(fd_telnet, " Starts: %s, Laufzeit: %s s, Laufzeit: %s h     \n",
	 read_starts(), read_runtime(), read_runtime_h() );
  dprintf(fd_telnet, " Leistung: %s %%     \n", read_power() );
  
  dprintf(fd_telnet, "HYDRAULIK:\n");
  dprintf(fd_telnet, " Ventilstellung Numerisch: %s, Stellung: %s       \n",
	 read_ventil_numeric(), read_ventil() );
  dprintf(fd_telnet, " Pumpe: %s %%        \n", read_pump_power() );
  dprintf(fd_telnet, " Volumenstrom: %s l/h        \n", read_flow() );
      
  dprintf(fd_telnet, "HEIZKREISTEMPERATUREN:\n");
  dprintf(fd_telnet, " Vorlaufsoll: %s °C, Raumsoll: %s °C, red. Raumsoll %s °C     \n",
	 read_VL_soll_temp(), read_raum_soll_temp(), read_red_raum_soll_temp() );
  dprintf(fd_telnet, " Neigung: %s,  Niveau: %s   \n", read_neigung(), read_niveau() );
  dprintf(fd_telnet, "\n");
}

// Wird einmalig aufgerufen, vorwiegend um das Telnet Socket zu initialisieren.
// Es muss darauf geachtet werden, dass masters_fds vor dem Aufruf gelöscht wird und nicht
// danach.
void telnet_init(fd_set *master_fds )
{
  /* create socket */
  if((fd_listener = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1)
    {
      fprintf( stderr, "Error creating Socket!\n");
      exit(1);
    }

  // Set REUSEADDR option
  // Sonst kann man nach einem Programmabbruch längere Zeit nicht neu starten
  const int optVal = 1;
  if ( setsockopt(fd_listener, SOL_SOCKET, SO_REUSEADDR, (void*) &optVal, sizeof(int)) == -1 )
    {
      fprintf( stderr, "Error setting Socket Options!\n");
      exit(1);
    }
  
  /* bind */
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = INADDR_ANY;
  serveraddr.sin_port = htons(PORT);
  memset(&(serveraddr.sin_zero), '\0', sizeof(serveraddr.sin_zero) );
  if(bind(fd_listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
      fprintf( stderr, "Error bind Socket: %s\n", strerror(errno));
      exit(1);
    }
  /* listen */
  if(listen(fd_listener, 5) == -1) // "5" = listen backlog
    {
      fprintf( stderr, "Error listen Socket!\n");
      exit(1);
    }
  else
    {
      fprintf( stderr, "Now listening to telnet Port %d\n", PORT );
    }
  
  /* add the listener to the master set */
  FD_SET(fd_listener, master_fds);
}

// Folgende Funktion wird periodisch nach der select() Rückkehr aufgerufen.
// Übergeben werden die Descriptormengen der aktuell aktiven Descriptoren
// (master_fds) und die von select() zurückgegebenen (read_fds).
// In master_fds werden ggf. neue Descriptoren eingetragen.
void telnet_task(fd_set *master_fds, fd_set *read_fds)
{
  static char telnet_buf[2048]; /* buffer for client data */
  static int telnet_buf_ptr;
  
  int fd_new;
  char command[80];
  int value;
  int i;
  
  // Gibt es eine neue Verbindungsanfrage?
  if ( FD_ISSET( fd_listener, read_fds ) )
    {
      socklen_t addrlen = sizeof(clientaddr); // wird als pointer gebraucht
      if((fd_new = accept(fd_listener, (struct sockaddr *)&clientaddr, &addrlen)) == -1)
	{
	  fprintf( stderr, "Error accepting Connection!\n");
	  exit(1);
	}
      else
	{
	  if ( fd_telnet == 0 )
	    { // Verbindung akzeptieren, wenn es noch keine gibt:
	      fd_telnet = fd_new;
	      FD_SET(fd_telnet , master_fds); /* add to master set */
	      printf("New Telnet connection from %s\n",
		     inet_ntoa(clientaddr.sin_addr) );
	      telnet_buf_ptr = 0; // Pufferzeiger initialisieren
	      dprintf( fd_telnet, "Welcome at vitalk, the Vitodens telnet Interface.\n");
	    }
	  else
	    { // Neue Verbindung ablehnen, wenn schon eine existiert:
	      printf("New Telnet connection declined.\n");
	      close( fd_new );
	    }
	}
    }

  // Gibt es Daten aus der Telnet Verbindung?
  if ( FD_ISSET( fd_telnet, read_fds) )
    {
      // Wir lesen hier immer nur ein Zeichen. Das ist zwar wohl nicht sehr
      // effizient aber vereinfacht das Pufferhandling erheblich.
      if( recv(fd_telnet, telnet_buf + telnet_buf_ptr, 1, 0 ) <= 0 )
	{ /* got error or connection closed by client */
	  close(fd_telnet); /* close it */
	  FD_CLR(fd_telnet, master_fds); /* remove from master set */
	  fd_telnet = 0; // Damit neue Verbindungen angenommen werden können
	  printf("Telnet Connection closed.\n");
	}
      else
	{ // Hier wurde ein Zeichen gelesen:
	  telnet_buf_ptr++;
	  telnet_buf[telnet_buf_ptr]='\0';
	  if ( ( strchr(telnet_buf, '\n') ) || // Newline im Puffer?
	       ( telnet_buf_ptr > sizeof(telnet_buf) - 4 ) ) // Oder Puffer ziemlich voll?
	    { // Dann werten wir das aus:
	      telnet_buf_ptr = 0; // Wird erst für neue Zeile wieder gebraucht
	      telnet_buf[75]='\0'; // Länger kann ein Befehl eh nicht sein
	      command[0] = '\0';
	      value = 0;
	      sscanf(telnet_buf, "%s %d", command, &value);
	      // Suche nach bekanntem Befehl:
	      for ( i=0; commands[i][0]; i++ ) 
		{
		  if ( strcmp( commands[i], command ) == 0 )
		    switch( i )
		      {
		      case 0:
		      case 1:
			print_help();
			break;
		      case 2:
		      case 3:
			print_all();
			break;
		      case 4:
		      case 5:
			dprintf(fd_telnet, "FEHLERSPEICHER:\n");
			dprintf(fd_telnet,  "%s", read_error_history() );
			dprintf(fd_telnet, "\n");
			break;
		      case 6:
			dprintf(fd_telnet, "FEHLERSPEICHER (numerisch):\n");
			dprintf(fd_telnet,  "%s", read_error_history_numeric() );
			dprintf(fd_telnet, "\n");
			break;
		      case 7:
			dprintf(fd_telnet, "Setting mode: ");
			if ( value >= 0 && value <= 2 )
			  {
			    write_mode_numeric( value );
			    dprintf(fd_telnet, "done.\n");
			  }
			else
			  dprintf(fd_telnet, "error!\n");
			break;
		      case 8:
			dprintf(fd_telnet, "Setting WW temp: ");
			if ( value >= 5 && value <= 60 )
			  {
			    write_WW_soll_temp( value );
			    dprintf(fd_telnet, "done.\n");
			  }
			else
			  dprintf(fd_telnet, "error!\n");
			break;
		      case 9:
			dprintf(fd_telnet, "Setting room temp: ");
			if ( value >= 10 && value <= 30 )
			  {
			    write_raum_soll_temp( value );
			    dprintf(fd_telnet, "done.\n");
			  }
			else
			  dprintf(fd_telnet, "error!\n");
			break;
		      case 10:
			dprintf(fd_telnet, "Setting reduced room temp: ");
			if ( value >= 10 && value <= 30 )
			  {
			    write_red_raum_soll_temp( value );
			    dprintf(fd_telnet, "done.\n");
			  }
			else
			  dprintf(fd_telnet, "error!\n");
			break;
		      }
		}
	    }
	}
    }
}