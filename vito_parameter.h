// Struktur zur Verwaltung der Parameter
struct s_parameter {
  const char * const p_name;         // Parameter Kurzname
  const char * const p_description;  // Beschreibung des Parameters
  const char * const p_einheit;      // Einheit (String)
  const int p_class;         // Parameterklasse, siehe #define
  const char * const (* const f_read) (void); // Funktion zum lesen aus der Vitodens
  const char * const (* const f_write) (const char *value_str); // Funktion zum Schreiben in die Vitodens
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
const char * const get_v( const char *name );
const char * const get_u( const char *name );
const char * const set_v( const char *name, const char *value );

