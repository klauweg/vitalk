// Struktur zur Verwaltung der Parameter
struct s_parameter {
  char *p_name;         // Parameter Kurzname
  char *p_description;  // Beschreibung des Parameters
  char *p_einheit;      // Einheit (String)
  int p_class;         // Parameterklasse, siehe #define
  int (*f_read) (char *valuestr);   // Funktion zum lesen aus der Vitodens
  int (*f_write) (char *valuestr);  // Funktion zum Schreiben in die Vitodens
};

// Parameter liste exportieren:
extern const struct s_parameter parameter_liste[];

// Parameterklassen:
#define P_ALLE       0
#define P_ALLGEMEIN  1
#define P_KESSEL     2
#define P_WARMWASSER 3
#define P_HEIZKREIS  4
#define P_BRENNER    5
#define P_HYDRAULIK  6

// Prototypen:
char * get_v( char *name );
char * get_u( char *name );

