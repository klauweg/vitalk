vitalk: vitalk.c  vito_io.c vito_parameter.c
	gcc -Wall -o vitalk vitalk.c vito_io.c vito_parameter.c

copy:
	scp *.c root@lupo:vitalk/
	scp *.h root@lupo:vitalk/
	scp Makefile root@lupo:vitalk/
	
