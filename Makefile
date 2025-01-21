CC := gcc
CC_FLAGS := -Wall -Wextra

shell: shell.c builtins.o
	$(CC) $(CC_FLAGS) -o shell shell.c builtins.o

builtins: builtins.c builtins.h 
	$(CC) $(CC_FLAGS) -c -o builtins.o builtins.c

clean:
	@-rm -f shell *.o

.PHONY: clean
