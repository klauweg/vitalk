// sed -e 's#^\(..\) \(.*\).#"\2", //\1#g;s#^\(..\)$#"", //\1#g;' roharray.txt >fehlerliste.h
char *fehlerliste[] = {
"Regelbetrieb (kein Fehler)", //00
"", //01
"", //02
"", //03
"", //04
"", //05
"", //06
"", //07
"", //08
"", //09
"", //0A
"", //0B
"", //0C
"", //0D
"", //0E
"Wartung (fuer Reset Codieradresse 24 auf 0 stellen)", //0F
"Kurzschluss Aussentemperatursensor", //10
"", //11
"", //12
"", //13
"", //14
"", //15
"", //16
"", //17
"Unterbrechung Aussentemperatursensor", //18
"", //19
"", //1A
"", //1B
"", //1C
"", //1D
"", //1E
"", //1F
"Kurzschluss Vorlauftemperatursensor", //20
"Kurzschluss Ruecklauftemperatursensor", //21
"", //22
"", //23
"", //24
"", //25
"", //26
"", //27
"Unterbrechung Aussentemperatursensor", //28
"Unterbrechung Ruecklauftemperatursensor", //29
"", //2A
"", //2B
"", //2C
"", //2D
"", //2E
"", //2F
"Kurzschluss Kesseltemperatursensor", //30
"", //31
"", //32
"", //33
"", //34
"", //35
"", //36
"", //37
"Unterbrechung Kesseltemperatursensor", //38
"", //39
"", //3A
"", //3B
"", //3C
"", //3D
"", //3E
"", //3F
"Kurzschluss Vorlauftemperatursensor M2", //40
"", //41
"Unterbrechung Vorlauftemperatursensor M2", //42
"", //43
"", //44
"", //45
"", //46
"", //47
"", //48
"", //49
"", //4A
"", //4B
"", //4C
"", //4D
"", //4E
"", //4F
"Kurzschluss Speichertemperatursensor", //50
"", //51
"", //52
"", //53
"", //54
"", //55
"", //56
"", //57
"Unterbrechung Speichertemperatursensor", //58
"", //59
"", //5A
"", //5B
"", //5C
"", //5D
"", //5E
"", //5F
"", //60
"", //61
"", //62
"", //63
"", //64
"", //65
"", //66
"", //67
"", //68
"", //69
"", //6A
"", //6B
"", //6C
"", //6D
"", //6E
"", //6F
"", //70
"", //71
"", //72
"", //73
"", //74
"", //75
"", //76
"", //77
"", //78
"", //79
"", //7A
"", //7B
"", //7C
"", //7D
"", //7E
"", //7F
"", //80
"", //81
"", //82
"", //83
"", //84
"", //85
"", //86
"", //87
"", //88
"", //89
"", //8A
"", //8B
"", //8C
"", //8D
"", //8E
"", //8F
"", //90
"", //91
"Solar: Kurzschluss Kollektortemperatursensor", //92
"Solar: Kurzschluss Sensor S3", //93
"Solar: Kurzschluss Speichertemperatursensor", //94
"", //95
"", //96
"", //97
"", //98
"", //99
"Solar: Unterbrechung Kollektortemperatursensor", //9A
"Solar: Unterbrechung Sensor S3", //9B
"Solar: Unterbrechung Speichertemperatursensor", //9C
"", //9D
"", //9E
"Solar: Fehlermeldung Solarteil (siehe Solarregler)", //9F
"", //A0
"", //A1
"", //A2
"", //A3
"", //A4
"", //A5
"", //A6
"Bedienteil defekt", //A7
"", //A8
"", //A9
"", //AA
"", //AB
"", //AC
"", //AD
"", //AE
"", //AF
"Kurzschluss Abgastemperatursensor", //B0
"Kommunikationsfehler Bedieneinheit", //B1
"", //B2
"", //B3
"Interner Fehler (Elektronik)", //B4
"Interner Fehler (Elektronik)", //B5
"Ungueltige Hardwarekennung (Elektronik)", //B6
"Interner Fehler (Kesselkodierstecker)", //B7
"Unterbrechung Abgastemperatursensor", //B8
"Interner Fehler (Dateneingabe wiederholen)", //B9
"Kommunikationsfehler Erweiterungssatz fuer Mischerkreis M2", //BA
"", //BB
"Kommunikationsfehler Fernbedienung Vitorol, Heizkreis M1", //BC
"Kommunikationsfehler Fernbedienung Vitorol, Heizkreis M2", //BD
"Falsche Codierung Fernbedienung Vitorol", //BE
"", //BF
"", //C0
"Externe Sicherheitseinrichtung (Kessel kuehlt aus)", //C1
"Kommunikationsfehler Solarregelung", //C2
"", //C3
"", //C4
"Kommunikationsfehler drehzahlgeregelte Heizkreispumpe, Heizkreis M1", //C5
"Kommunikationsfehler drehzahlgeregelte Heizkreispumpe, Heizkreis M2", //C6
"Falsche Codierung der Heizkreispumpe", //C7
"", //C8
"Stoermeldeeingang am Schaltmodul-V aktiv", //C9
"", //CA
"", //CB
"", //CC
"Kommunikationsfehler Vitocom 100 (KM-BUS)", //CD
"Kommunikationsfehler Schaltmodul-V", //CE
"Kommunikationsfehler LON Modul", //CF
"", //D0
"Brennerstoerung", //D1
"", //D2
"", //D3
"Sicherheitstemperaturbegrenzer hat ausgeloest oder Stoermeldemodul nicht richtig gesteckt", //D4
"", //D5
"", //D6
"", //D7
"", //D8
"", //D9
"Kurzschluss Raumtemperatursensor, Heizkreis M1", //DA
"Kurzschluss Raumtemperatursensor, Heizkreis M2", //DB
"", //DC
"Unterbrechung Raumtemperatursensor, Heizkreis M1", //DD
"Unterbrechung Raumtemperatursensor, Heizkreis M2", //DE
"", //DF
"", //E0
"", //E1
"", //E2
"", //E3
"Fehler Versorgungsspannung", //E4
"Interner Fehler (Ionisationselektrode)", //E5
"Abgas- / Zuluftsystem verstopft", //E6
"", //E7
"", //E8
"", //E9
"", //EA
"", //EB
"", //EC
"", //ED
"", //EE
"", //EF
"Interner Fehler (Regelung tauschen)", //F0
"Abgastemperaturbegrenzer ausgeloest", //F1
"Temperaturbegrenzer ausgeloest", //F2
"Flammensigal beim Brennerstart bereits vorhanden", //F3
"Flammensigal nicht vorhanden", //F4
"", //F5
"", //F6
"Differenzdrucksensor defekt", //F7
"Brennstoffventil schliesst zu spaet", //F8
"Geblaesedrehzahl beim Brennerstart zu niedrig", //F9
"Geblaesestillstand nicht erreicht", //FA
"", //FB
"", //FC
"Fehler Gasfeuerungsautomat", //FD
"Starkes Stoerfeld (EMV) in der Naehe oder Elektronik defekt", //FE
"Starkes Stoerfeld (EMV) in der Naehe oder interner Fehler" //FF
};
