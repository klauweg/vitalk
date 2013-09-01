#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>

// Globals:
static int fd_tty = 0; // Filedescriptor serielle Schnittstelle
extern int frame_debug;

// Keine Rückgabewerte. Wenns nicht geht wird sowieso beendet:
void opentty(char *device)
{
   struct termios my_termios;
   int modemctl;
   
   // Open modem device for reading and writing and not as controlling tty
   // because we don't want to get killed if linenoise sends CTRL-C.
   if ( ( fd_tty = open( device, O_RDWR | O_NOCTTY)) < 0 )
     {
	fprintf( stderr, "Fehler beim Oeffnen von %s: %s\n", device, strerror(errno));
	exit(1);
     }

   // termios Struktur "loeschen"
   bzero( &my_termios, sizeof(my_termios) );
   
   // .. und konfigurieren:
   my_termios.c_iflag = IGNBRK | IGNPAR;
   my_termios.c_oflag = 0;
   my_termios.c_lflag = 0;
   my_termios.c_cflag = (CLOCAL | HUPCL | B4800 | CS8 | CREAD | PARENB | CSTOPB);
   my_termios.c_cc[VMIN] = 0;
   my_termios.c_cc[VTIME] = 50; // timeout fuer tty RX
     
   if ( tcsetattr(fd_tty, TCSAFLUSH, &my_termios) < 0 )
     {
	fprintf( stderr, "Fehler beim Konfigurieren von %s: %s\n", device, strerror(errno));
	exit(1);
     }
   
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

// Schnittstelle schließen
void closetty( void )
{
  close( fd_tty );
}

// Keine Rückgabewerte. Wenns nicht geht wird sowieso beendet:
// Initialisieren des 300 Protokolls:
void vito_init( void )
{
   int trys;
   unsigned char rec;
   const unsigned char initKw[] = { 0x04 };
   const unsigned char initSeq[] = { 0x16, 0x00, 0x00 };
   
   // So lange 0x04 senden bis 0x05 empfangen wird:
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
	write( fd_tty, initKw, 1 );
	sleep( 1 );
	tcflush( fd_tty, TCIFLUSH );
	read( fd_tty, &rec, 1 );
     }
   while ( rec != 0x05 );
   fprintf( stderr, "Success.\n");
   
   // Wenn 0x05 empfangen wurde, Proto 300 Init:
   fprintf( stderr, "Try Proto 300 Init: " );
   write( fd_tty, initSeq, 3 );
   read( fd_tty, &rec, 1 );
   if ( rec != 0x06 )
     {
	fprintf( stderr, "failed!\n" );
	exit(1);
     }

   fprintf( stderr, "Success.\n");
}

// Zurückschalten ins KW Protokoll:
void vito_close( void )
{
   const unsigned char initKw[] = { 0x04 };
   
   write( fd_tty, initKw, 1 );
}

// CRC Berechnung
static int calcCRC( unsigned char *buffer )
{
  int crc = 0;
  int i;
  
  for ( i = 1; i <= buffer[1] + 1; i++)
    crc += buffer[i];
    
  crc &= 0xff;
  
  return crc;
}

// Debug Ausgabe Array als Hexadezimal:
static int print_hex( unsigned char *buffer, int len )
{
  int i;
  
  fprintf( stderr, "Data:" );
  for ( i = 0; i < len; i++ )
    fprintf( stderr, " 0x%02x", buffer[i] );
  fprintf( stderr, "\n" );
  // Beispiel:
  // Senden 41 05 00 01 55 25 02 82
  // Empfangen 06 41 07 01 01 55 25 02 07 01 8D
}

// Lowlevel Kommunikation mit der Vitodens:
// Reine Nutzdaten ohne Telegrammlängenfeld und CRC werden übergeben:
// (Wird für Schreib- und Lesezugriff benutzt)
static int vito_meeting( unsigned char *tx_data, int tx_data_len, 
			 unsigned char *rx_data )
{
  unsigned char buffer[300];
  unsigned char rec;
  int rx_data_len;
  int i;
  
  // Construct TX Frame:
  buffer[0] = 0x41;     // Start of Frame
  buffer[1] = tx_data_len;  // Länge der Nutzdaten eintragen
  memcpy( &buffer[2], tx_data, tx_data_len ); // Nutzdaten kopieren
  buffer[tx_data_len+2] = calcCRC( buffer ); // CRC berechnen

  // Debug output
  if (frame_debug)
    print_hex( buffer, tx_data_len+3 );

  // Zur Sicherheit:
  if ( fd_tty == 0 )
    {
      fprintf( stderr, "No tty available!\n" );
      return -1;
    }
  
  //////////// Anfrage zur Vitodens senden:
  tcflush( fd_tty, TCIOFLUSH );
  if ( write( fd_tty, command, tx_data_len+3 ) < tx_data_len+3 ) // payload + overhead
    {
      fprintf( stderr, "Write to tty failed.\n" );
      return -1;
    }
  
  //////////// Antwort verarbeiten:
  // Got ACK?
  if ( read( fd_tty, &rec, 1 ) < 1 )
    {
      fprintf( stderr, "No ACK on Transmission! (got nothing)\n" );
      return -1;
    }
  if ( rec != 0x06 )
    {
      fprintf( stderr, "No ACK on Transmission! (got %x)\n", rec );
      return -1;
    }
  // Got Answer Frame Start?
  if ( read( fd_tty, &buffer[0], 1 ) < 1 )
    {
      fprintf( stderr, "No Frame Start! (got nothing)\n" );
      return -1;
    }
  if ( buffer[0] != 0x41 )
    {
      fprintf( stderr, "No Frame Start! (got %x)\n", buffer[0] );
      return -1;
    }
  // Telegrammlänge empfangen:
  if ( read( fd_tty, &buffer[1], 1 ) < 1 )
    {
      fprintf( stderr, "No Frame Size received!\n" );
      return -1;
    }
  rx_data_len = buffer[1];
  // Payload + CRC empfangen:
  for ( i = 0; i <= rx_data_len; i++ )
    if ( read( fd_tty, &buffer[i+2], 1 ) < 1 )
      {
        fprintf( stderr, "Answer Frame too short! (received %d bytes of %d expected):\n", i, rx_data_len+1 );
	print_hex( buffer, i+2 );
	return -1;
      }

  // Debug output
  if (frame_debug)
    print_hex( rx_buffer, rx_data_len + 3 );
  
// hier fehlt noch was!
}
    

  
  
  
// Speicherbereich von Vitodens anfragen:
int vito_request( int location, int size, unsigned char *vitomem )
{
  unsigned char command[20];
  int result;
  
  // Hier wird das Anfragetelegramm gebastelt:
  command[0] = 0x00;    // Type of Message: Anfrage
  command[1] = 0x01;    // Lesezugriff
  command[2] = (location >> 8) & 0xff; // high byte
  command[3] = location & 0xff; // low byte
  command[4] = size;    // Anzahl der angeforderten Bytes
  result = vito_meeting( command, 5, command );

  
}

   