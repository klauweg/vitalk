// Ausführen einer SQL Query
int my_query( char * querystring, 
              char * hostname,
	      char * username,
	      char * password,
	      char * database );

// einen Datensatz in die Datenbank schreiben:
void my_live_log ( void );

// Globale Variable zu Konfiguration:
extern char *my_hostname;
extern char *my_username;
extern char *my_password;
extern char *my_database;

