IDIR =
CC=gcc
CFLAGS= -c


SOURCES = ./udp_server.c
OBJ_FILES = $(SOURCES:.c=.o)
EXECUTABLE = server

.PHONY : all

%.o:%.c
	$(CC) $(CFLAGS) -o $@ $< 

$(EXECUTABLE) : $(OBJ_FILES)
	$(CC) $^ -o $@

clean:
	rm -rf ./*.o server 

