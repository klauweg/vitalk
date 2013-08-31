#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>

// Filedescriptor für tty global:
int fd_tty;

// Keine Rückgabewerte. Wenns nicht geht wird sowieso beendet:
void opentty(char *device)
{
   struct termios my_termios;
   int modemctl;
   
   // Open modem device for reading and writing and not as controlling tty
   // because we don't want to get killed if linenoise sends CTRL-C.
   if ( ( fd_tty = open( device, O_RDWR | O_NOCTTY)) < 0 )
     {
	fprintf( stderr, "Fehler beim oeffnen von %s: %s\n", device, strerror(errno));
	exit(1);
     }

   // termios Struktur "loeschen"
   bzero( &my_termios, sizeof(my_termios) );
   
   my_termios.c_iflag = IGNBRK | IGNPAR;
   my_termios.c_oflag = 0;
   my_termios.c_lflag = 0;
   my_termios.c_cflag = (CLOCAL | HUPCL | B4800 | CS8 | CREAD | PARENB | CSTOPB);
   my_termios.c_cc[VMIN] = 0;
   my_termios.c_cc[VTIME] = 50; // timeout fuer tty RX
     
   tcsetattr(fd_tty, TCSAFLUSH, &my_termios);
   
   // DTR Leitung aktivieren:
   modemctl = 0;
   ioctl( fd_tty, TIOCMGET, &modemctl );
   modemctl |= TIOCM_DTR;
   if ( ioctl(fd_tty,TIOCMSET,&modemctl) < 0 )
     {
	fprintf( stderr, "Fehler beim Aktivieren von DTR %s: %s\n", device, strerror(errno));
	exit(1);
     }
}

// Keine Rückgabewerte. Wenns nicht geht wird sowieso beendet:
void vito_init( void )
{
   int trys;
   char rec;
   
   trys = 5;
   fprintf( stderr, "Reset Communication to KW Proto." );
   do
     {
	if ( trys == 0 )
	  {
	     fprintf( stderr, "failed!\n" );
	     exit(1);
	  }
	trys--;
	fprintf( stderr, "." );
	tcflush( fd_tty, TCIOFLUSH );
	write( fd_tty, 0x04, 1 );
	sleep( 1 );
	tcflush( fd_tty, TCIFLUSH );
	read( fd_tty, &rec, 1 );
     }
   while ( rec != 0x05 );
   fprintf( stderr, "Success.\n");
   
   fprintf( stderr, "Try Proto 300 Init: " );
   write( fd_tty, { 0x16, 0x00, 0x00 }, 3 );
   read( fd_tty, &rec, 1 );
   if ( rec != 0x06 )
     {
	fprintf( stderr, "failed!\n" );
	exit(1);
     }

   fprintf( stderr, "Success.\n");
}

void vito_close( void )
{
   write( fd_tty, 0x04, 1 );
}





main()
{
   opentty("/dev/ttyUSB0");
   vito_init();
   sleep(3);
}

   