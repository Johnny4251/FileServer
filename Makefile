CC=gcc
CFLAGS=-Wall -g

all: webserver client

webserver: webserver.c status_codes.c
	$(CC) $(CFLAGS) webserver.c status_codes.c -o webserver

client: client.c
	$(CC) $(CFLAGS) client.c -o client

clean:
	rm -f webserver client

