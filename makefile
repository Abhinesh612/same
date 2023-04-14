CC=clang
CFLAG=-g -Wall -Wextra -pedantic -Werror -std=c11
EXEC=same

SOURCES=$(wildcard ./*.c)
OBJECTS=$(SOURCES:.c=.o)

$(EXEC) : $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC)

%.o : %.c
	$(CC) -c $(CFLAG) $< -o $@

.PHONY: clean
clean:
	rm -f $(EXEC) $(OBJECTS)
