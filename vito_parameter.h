// Struktur zur Verwaltung der Parameter
struct s_parameter {
  char *p_name;         // Parameter Kurzname
  char *p_description;  // Beschreibung des Parameters
  char *p_einheit;      // Einheit (String)
  int p_class;         // Parameterklasse, siehe #define
  int (*f_read) (char **value_ptr); // Funktion zum lesen aus der Vitodens
  int (*f_write) (char *value_str); // Funktion zum Schreiben in die Vitodens
};

// Parameter liste exportieren:
extern const struct s_parameter parameter_liste[];

// Parameterklassen:
#define P_ALLE       0
#define P_ERRORS     1
#define P_ALLGEMEIN  2
#define P_KESSEL     3
#define P_WARMWASSER 4
#define P_HEIZKREIS  5
#define P_BRENNER    6
#define P_HYDRAULIK  7

// Prototypen:
char * get_v( char *name );
char * get_u( char *name );
char * set_v( char *name, char *value );

