#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

// Globals:
static int fd_tty = 0; // Filedescriptor serielle Schnittstelle
static int errorcount = 0; // Abbruch bei zu häufigen Fehlern
int frame_debug = 0; // Ausgabe der Framedaten als hex auf stderr wenn =1

// Öffnen der seriellen Schnittstelle:
// Keine Rückgabewerte. Wenns nicht geht wird sowieso beendet:
void opentty(char *device)
{
   struct termios my_termios;
   int modemctl;
   
   // Open modem device for reading and writing and not as controlling tty
   // because we don't want to get killed if linenoise sends CTRL-C.
   // Wird das nicht auch per termios konfiguriert? Naja, kann ja nicht schaden.
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
   // Mein Interface braucht das eigentlich nicht zur Stromversorgung aber die
   // Statusled zeigt dann zumindest eine aktive Schnittstelle an.
   modemctl = 0;
   ioctl( fd_tty, TIOCMGET, &modemctl );
   modemctl |= TIOCM_DTR;
   if ( ioctl(fd_tty,TIOCMSET,&modemctl) < 0 )
     {
	fprintf( stderr, "Fehler beim Aktivieren von DTR %s: %s\n", device, strerror(errno));
	exit(1);
     }
}

// Serielle schnittstelle schließen
void closetty( void )
{
  close( fd_tty );
}

// Keine Rückgabewerte. Wenns nicht geht wird sowieso beendet:
// Initialisieren des 300 Protokolls:
void vito_init( void )
{
   int trys;
   uint8_t rec;
   const uint8_t initKw[] = { 0x04 };
   const uint8_t initSeq[] = { 0x16, 0x00, 0x00 };
   
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
	read( fd_tty, &rec, 1 ); // Wenn KW Protokoll aktiv, muss irgendwann 0x05 kommen! Sonst Timeout
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
   const uint8_t initKw[] = { 0x04 };

  // Hier etwas unklar wie man sich verhalten soll wenn der
  // Abbruch mitten in einer Telegrammübertragung erfolgt.
  // Etwas warten kann jedenfalls nicht schaden.
  // Bis jetzt hat das immer recht zuverlässig funktioniert.
   write( fd_tty, initKw, 1 );
}

// 8-bit CRC Berechnung
// Übergeben wird ein komplettes Frame einschließlich Startbyte
// (0x41). Das Startbyte wird aber nicht für die CRC Berechnung berücksichtigt.
// Rückgabewert: 8-bit CRC
static uint8_t calcCRC( uint8_t *buffer )
{
  int crc = 0;
  int i;
  
  for ( i = 1; i <= buffer[1] + 1; i++)
    crc += buffer[i];

  return (crc & 0xff);
}

// Debug Ausgabe: Array als Hexadezimal:
static void print_hex( uint8_t *buffer, int len )
{
  int i;
  
  fprintf( stderr, "Data:" );
  for ( i = 0; i < len; i++ )
    fprintf( stderr, " 0x%02x", buffer[i] );
  fprintf( stderr, "\n" );
}

// Lowlevel Kommunikation mit der Vitodens:
// Es wird pro Aufruf immer ein Telegramm gesendet und eines empfangen.
// Dabei kann es sich um Schreib- oder Lesezugriffe handeln. Diese Funktion
// wird aber von den Nutzdaten bestimmt.
// Als rx_data und tx_data werden reine Nutzdaten übergeben ohne
// das Telegrammlängenfeld und die CRC Prüfsumme.
// tx_data_len und rx_data_len sind Payloadlängen! Keine Speicherbereichsgrößen
// des Regelungsadressraumes!
// Rückgabewert: rx_data_len ( = -1 im Fehlerfall)
static int vito_meeting( uint8_t *tx_data, int tx_data_len, uint8_t *rx_data )
{
  uint8_t buffer[200];
  uint8_t rec;
  int rx_data_len;
  int i;
  
  if ( tx_data_len > 100 )
    { // Das sollte ja eigentlich nicht vorkommen, aber sicher ist sicher,
      // denn das Datenfeld im Telegramm für die Payload-Länge hat ja auch nur 8 bit!
      fprintf( stderr, "TX: Payload too large!\n" );
      exit( 1 );
    }
  
  // Construct TX Frame:
  buffer[0] = 0x41;     // Start of Frame
  buffer[1] = tx_data_len;  // Länge der Nutzdaten eintragen
  memcpy( &buffer[2], tx_data, tx_data_len ); // Nutzdaten kopieren
  buffer[tx_data_len+2] = calcCRC( buffer ); // CRC berechnen

  // Debug output
  if (frame_debug)
    {
      fprintf( stderr, "TX: " );
      print_hex( buffer, tx_data_len+3 );
    }
  
  // Zur Sicherheit:
  // Aber wenn das wirklich schief geht, kann man das Programm auch gleich beenden.
  if ( fd_tty == 0 )
    {
      fprintf( stderr, "TX: No tty available!\n" );
      exit( 1 );
    }
  
  //////////// Anfrage zur Vitodens senden:
  tcflush( fd_tty, TCIOFLUSH );
  if ( write( fd_tty, buffer, tx_data_len+3 ) < tx_data_len+3 ) // payload + overhead
    {
      fprintf( stderr, "TX: Write to tty failed.\n" );
      return -1;
    }
  
  //////////// Antwort verarbeiten:
  // Got ACK?
  if ( read( fd_tty, &rec, 1 ) < 1 )
    { // Timeout
      fprintf( stderr, "RCVD No ACK on Transmission! (got nothing)\n" );
      return -1;
    }
  if ( rec != 0x06 )
    { // Es wurde ein Byte empfangen, aber kein ACK:
      fprintf( stderr, "RCVD No ACK on Transmission! (got 0x%02x)\n", rec );
      if ( rec == 0x15 )
	fprintf( stderr, "CRC ERROR REPORTED BY VITODENS(TX)!\n" );
      return -1;
    }
  
  // Got Answer Frame Start?
  if ( read( fd_tty, &buffer[0], 1 ) < 1 )
    { // Timeout
      fprintf( stderr, "RCVD No Frame Start! (got nothing)\n" );
      return -1;
    }
  if ( buffer[0] != 0x41 )
    { // Es wurde ein Byte empfangen, aber kein Frame Start:
      fprintf( stderr, "RCVD No Frame Start! (got 0x%02x)\n", buffer[0] );
      return -1;
    }
  
  // Telegrammlänge empfangen:
  if ( read( fd_tty, &buffer[1], 1 ) < 1 )
    { // Timeout
      fprintf( stderr, "RCVD No Frame Size!\n" );
      return -1;
    }
  rx_data_len = buffer[1];
  
  // Payload + CRC empfangen:
  for ( i = 0; i < rx_data_len+1; i++ )
    if ( read( fd_tty, &buffer[i+2], 1 ) < 1 )
      {
        fprintf( stderr, "Answer Frame too short! (received %d bytes of %d expected)\n", i, rx_data_len+1 );
	print_hex( buffer, i+2 );
	return -1;
      }

  // Debug output
  if (frame_debug)
    {
      fprintf( stderr, "RX: " );
      print_hex( buffer, rx_data_len + 3 );
    }
  
  // CRC prüfen
  if ( calcCRC(buffer) != buffer[rx_data_len + 2] )
    {
      fprintf( stderr, "Bad CRC on RX: " );
      print_hex( buffer, rx_data_len + 3 );
      return -1;
    }
  
  memcpy( rx_data, &buffer[2], rx_data_len );
  return rx_data_len;
}
    

// Speicherbereich von Vitodens lesen:
// Übergeben werden Addresse und Größe des zu lesenden
// Speicherbereichs aus der Regelung sowie ein Pointer
// auf die Zieladresse im Speicher:
// Rückgabewert: -1 = Fehler
//                0 = OK
int vito_read( int location, int size, uint8_t *vitomem )
{
  uint8_t command[20];
  uint8_t result[300];
  int result_len;
  int flag;
  
  // Hier werden die Anfrage Nutzdaten gebastelt:
  command[0] = 0x00;    // Type of Message: Host -> Vitodens
  command[1] = 0x01;    // Lesezugriff
  command[2] = (location >> 8) & 0xff; // high byte // Adresse ist BIG-ENDIAN!
  command[3] = location & 0xff; // low byte
  command[4] = size;    // Anzahl der angeforderten Bytes

  // Anfrage ausführen:
  result_len = vito_meeting( command, 5, result );
  
  // Fehler von der vito_meeting() Funktion übernehmen:
  if ( result_len < 0 )
    {
      fprintf( stderr, "READ: failed. (error in meeting)\n" );
      bzero( vitomem, size );
      if ( errorcount++ > 10 ) exit(2);
      return -1;
    }
  
  // Wir untersuchen die empfangene Payload auf plausibilität:
  flag = 0;
  if ( result_len != 5 + size)
    {
      fprintf( stderr, "READ: Wrong RCVD Payload length!\n" );
      flag = 1;
    }
  if ( (command[2] != result[2]) || (command[3] != result[3]) )
    {
      fprintf( stderr, "READ: Wrong Address received!\n" );
      flag = 1;
    }
  if ( result[0] != 0x01 )
    {
      fprintf( stderr, "READ: Wrong Message Type received!\n" );
      flag = 1;
    }
  if ( result[1] != 0x01 )
    {
      fprintf( stderr, "READ: Wrong Access Type received!\n" );
      flag = 1;
    }
  if ( command[4] != result[4] )
    {
      fprintf( stderr, "READ: Wrong Memory Size received!\n" );
      flag = 1;
    }
  
  if (flag)
    {
      fprintf( stderr, "READ: Received Payload " );
      print_hex( result, result_len );
      bzero( vitomem, size );
      if ( errorcount++ > 10 ) exit(2);
      return -1;
    }
  
  // Zu lesenden Speicherbereich aus der empfangenen Payload
  // kopieren:
  memcpy( vitomem, &result[5], size );
  errorcount = 0;
  return 0;
}

// Speicherbereich an Vitodens schreiben:
// Übergeben werden Addresse und Größe des zu schreibenden
// Speicherbereichs in der Regelung sowie ein Pointer
// auf die Quelladresse im Speicher:
// Rückgabewert: -1 = Fehler
//                0 = OK
int vito_write( int location, int size, uint8_t *vitomem )
{
  uint8_t command[200];
  uint8_t result[200];
  int result_len;
  int flag;

  // Hier werden die Anfrage Nutzdaten gebastelt:
  command[0] = 0x00;    // Type of Message: Host -> Vitodens
  command[1] = 0x02;    // Schreibzugriff
  command[2] = (location >> 8) & 0xff; // high byte Adresse // ADRESSE IST BIG-ENDIAN!
  command[3] = location & 0xff; // low byte Adresse
  command[4] = size;    // Anzahl der zu schreibenden Bytes
  memcpy( &command[5], vitomem, size );
  
  // Anfrage ausführen:
  result_len = vito_meeting( command, 5 + size, result );
  
  // Fehler von der vito_meeting() Funktion
  if ( result_len < 0 )
    {
      fprintf( stderr, "WRITE: failed. (error in meeting)\n" );
      if ( errorcount++ > 10 ) exit(2);
      return -1;
    }
  
  // Wir untersuchen die empfangene Payload auf plausibilität:
  flag = 0;
  if ( result_len != 5 )
    {
      fprintf( stderr, "WRITE: Wrong RCVD Payload length!\n" );
      flag = 1;
    }
  if ( (command[2] != result[2]) || (command[3] != result[3]) )
    {
      fprintf( stderr, "WRITE: Wrong Adress received!\n" );
      flag = 1;
    }
  if ( result[0] != 0x01 )
    {
      fprintf( stderr, "WRITE: Wrong Message Type received!\n" );
      flag = 1;
    }
  if ( result[1] != 0x02 )
    {
      fprintf( stderr, "WRITE: Wrong Access Type received!\n" );
      flag = 1;
    }
  if ( command[4] != result[4] )
    {
      fprintf( stderr, "WRITE: Wrong Memory Size received!\n" );
      flag = 1;
    }
  
  if (flag)
    {
      fprintf( stderr, "WRITE: Received Payload " );
      print_hex( result, result_len );
      if ( errorcount++ > 10 ) exit(2);
      return -1;
    }

  errorcount = 0;
  return 0;
}


