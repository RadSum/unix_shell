CC := gcc
CC_FLAGS := -Wall -Wextra -pedantic

shell: shell.c builtins.o parsing.o
	$(CC) $(CC_FLAGS) -o shell shell.c builtins.o parsing.o

builtins: builtins.c builtins.h 
	$(CC) $(CC_FLAGS) -c -o builtins.o builtins.c

parsing: parsing.c parsing.h 
	$(CC) $(CC_FLAGS) -c -o parsing.o parsing.c

clean:
	@-rm -f shell *.o

.PHONY: clean
