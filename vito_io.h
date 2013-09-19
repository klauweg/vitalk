#include <stdint.h>

extern int frame_debug;
extern time_t vito_keepalive;

// Öffnet die serielle Schnittstelle zum Kessel:
void opentty(char *device);

// Schließt die serielle Schnittstelle zum Kessel:
void closetty( void );

// Initialisiert das 300 Protokoll mit dem Kessel:
void vito_init( void );

// Schaltet den Kessel in das KW Protkoll zurück:
void vito_close( void );

// Liest Daten von der Vitodens:
int vito_read( int location, int size, uint8_t *vitomem );

// Schreibt Daten an die Vitodens:
int vito_write( int location, int size, uint8_t *vitomem );

