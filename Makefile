vitalk: vitalk.c vito_io.c vito_io.h vito_parameter.c vito_parameter.h telnet.c telnet.h
	gcc -Wall -o vitalk \
	    vitalk.c vito_io.c vito_parameter.c telnet.c

copy:
	scp *.c root@lupo:vitalk/
	scp *.h root@lupo:vitalk/
	scp Makefile root@lupo:vitalk/

heizung:
	scp *.c root@heizung:vitalk/
	scp *.h root@heizung:vitalk/
	scp Makefile root@heizung:vitalk/

