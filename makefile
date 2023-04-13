CC=gcc
CFLAG=-g -ggdb -Wall -Wextra -pedantic -std=c11
EXEC=same

SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)

$(EXEC) : $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC)

%.o : %.c
	$(CC) -c $(CFLAG) $< -o $@

clean:
	rm -f $(EXEC) $(OBJECTS)
