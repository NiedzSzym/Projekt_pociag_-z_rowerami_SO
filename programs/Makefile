# Compiler
CC = gcc

# Flags
CFLAGS = -Wall -Wextra -std=c11

# Source files
SRC = Main.c Pasazer.c Kierownik_pociagu.c Zawiadowca_stacji.c

# Headers
HEADERS = Funkcje.h

# Executables (one per source file)
EXE = $(SRC:.c=)

# Default target
all: $(EXE)

# Pattern rule to compile each source file into an executable
%: %.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $<

# Clean up
clean:
	rm -f $(EXE)