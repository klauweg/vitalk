# Für Spalten mit NULL wird zusätzlich Speicher benötigt! wir verzichten
# daher darauf.
# MyISAM Tabellen dürften für unseren Zweck effizienter sein.

create table vitolog_config ( timestamp timestamp
                       mode TINYINT(1),
		       warmwasser_soll TINYINT(2),
		       runtime INT,
		       raum_soll TINYINT(2),         # 1°
		       raum_soll_red TINYINT(2),     # 1°
		       neigung TINYINT,              # 0,1
		       niveau TINYINT,               # 1°

# Hier wird nur geloggt, wenn der Brenner oder pumpe auch läuft, aber dafür
# mit hoher Auflösung:
CREATE TABLE IF NOT EXISTS vitolog_kessel (
               timestamp timestamp DEFAULT CURRENT_TIMESTAMP NOT NULL PRIMARY KEY,  # 4byte
	       start_count INT UNSIGNED NOT NULL,           # Zähler (4byte)
	       ventil TINYINT(1) UNSIGNED NOT NULL,         # 1-3
	       pumpe TINYINT(3) UNSIGNED NOT NULL,          # %
	       flow SMALLINT UNSIGNED NOT NULL,             # l/h
	       leistung TINYINT(3) UNSIGNED NOT NULL,       # %
	       kessel_soll SMALLINT(3) NOT NULL,            # 0,1 °C
	       kessel_ist SMALLINT(3) NOT NULL,             # 0,1 °C
	       abgas SMALLINT(3) NOT NULL,                  # 0,1 °C
	       INDEX( start_count )
	    ) ENGINE = MyISAM, ROW_FORMAT=FIXED;
	    


# Hier gibts die Daten der ca. letzten Stunde in hoher zeitlicher Auflösung:
create table vitolog_live ( timestamp timestamp,     # default: current_timestamp
                       kessel_soll SMALLINT(3),
		       kessel_ist SMALLINT(3),       # 0,1
		       kessel_abgas SMALLINT(3),     # 0,1
		       warmwasser_offset TINYINT(2),
		       warmwasser_ist SMALLINT(3),   # 0,1
		       aussentemp SMALLINT(3),       # 0,1
		       aussentemp_ged SMALLINT(3),   # 0,1
		       starts INT,
		       leistung TINYINT(3),           # %
		       ventil TINYINT(1),
		       pumpe TINYINT(3),             # %
		       flow SMALLINT,                # l/h
		       vorlauf_soll TINYINT(2),      # 1°
		       error1 TINYINT,
		       error2 TINYINT,
		       error3 TINYINT,
		       error4 TINYINT,
		       error5 TINYINT,
		       error6 TINYINT,
		       error7 TINYINT,
		       error8 TINYINT,
		       error9 TINYINT,
		       error10 TINYINT
		       );

# Hier gibts die Daten der ca. letzten Stunde in hoher zeitlicher Auflösung:
create table vitolog_live ( timestamp timestamp,     # default: current_timestamp
                       mode TINYINT(1),
                       kessel_soll SMALLINT(3),
		       kessel_ist SMALLINT(3),       # 0,1
		       kessel_abgas SMALLINT(3),     # 0,1
		       warmwasser_soll TINYINT(2),
		       warmwasser_offset TINYINT(2),
		       warmwasser_ist SMALLINT(3),   # 0,1
		       aussentemp SMALLINT(3),       # 0,1
		       aussentemp_ged SMALLINT(3),   # 0,1
		       starts INT,
		       runtime INT,
		       leistung TINYINT(3),           # %
		       ventil TINYINT(1),
		       pumpe TINYINT(3),             # %
		       flow SMALLINT,                # l/h
		       vorlauf_soll TINYINT(2),      # 1°
		       raum_soll TINYINT(2),         # 1°
		       raum_soll_red TINYINT(2),     # 1°
		       neigung TINYINT,              # 0,1
		       niveau TINYINT,               # 1°
		       error1 TINYINT,
		       error2 TINYINT,
		       error3 TINYINT,
		       error4 TINYINT,
		       error5 TINYINT,
		       error6 TINYINT,
		       error7 TINYINT,
		       error8 TINYINT,
		       error9 TINYINT,
		       error10 TINYINT
		       );

# Hier schreiben wir die Daten nur sehr selten (ca. eine Stunde?)
create table vitolog_archiv ( timestamp timestamp,     # default: current_timestamp
                       mode TINYINT(1),
                       kessel_soll SMALLINT(3),
		       kessel_ist SMALLINT(3),       # 0,1
		       kessel_abgas SMALLINT(3),     # 0,1
		       warmwasser_soll TINYINT(2),
		       warmwasser_offset TINYINT(2),
		       warmwasser_ist SMALLINT(3),   # 0,1
		       aussentemp SMALLINT(3),       # 0,1
		       aussentemp_ged SMALLINT(3),   # 0,1
		       starts INT,
		       runtime INT,
		       leistung TINYINT(3),           # %
		       ventil TINYINT(1),
		       pumpe TINYINT(3),             # %
		       flow SMALLINT,                # l/h
		       vorlauf_soll TINYINT(2),      # 1°
		       raum_soll TINYINT(2),         # 1°
		       raum_soll_red TINYINT(2),     # 1°
		       neigung TINYINT,              # 0,1
		       niveau TINYINT,               # 1°
		       error1 TINYINT,
		       error2 TINYINT,
		       error3 TINYINT,
		       error4 TINYINT,
		       error5 TINYINT,
		       error6 TINYINT,
		       error7 TINYINT,
		       error8 TINYINT,
		       error9 TINYINT,
		       error10 TINYINT
		       );


# Hier werden die Daten zwar in hoher Zeitauflösung und auch langfristig gespeichert, aber
# nur die interessanten Werte:
create table vitolog_hires ( timestamp timestamp,     # default: current_timestamp
                       mode TINYINT(1),
                       kessel_soll SMALLINT(3),
		       kessel_ist SMALLINT(3),       # 0,1
		       kessel_abgas SMALLINT(3),     # 0,1
		       warmwasser_soll TINYINT(2),
		       warmwasser_offset TINYINT(2),
		       warmwasser_ist SMALLINT(3),   # 0,1
		       aussentemp SMALLINT(3),       # 0,1
		       aussentemp_ged SMALLINT(3),   # 0,1
		       starts INT,
		       runtime INT,
		       leistung TINYINT(3),           # %
		       ventil TINYINT(1),
		       pumpe TINYINT(3),             # %
		       flow SMALLINT,                # l/h
		       vorlauf_soll TINYINT(2),      # 1°
		       );

