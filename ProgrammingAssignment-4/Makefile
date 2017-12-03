CC=gcc
CFLAGS = -g 


all: webproxy
LDLIBS = -lcrypto

webproxy : webproxy.o 
	$(CC) -o webproxy webproxy.o $(LDLIBS)

webproxy.o: webproxy.c
 
clean:
	rm -f webproxy webproxy.o
