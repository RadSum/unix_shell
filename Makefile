CC := gcc
CC_FLAGS := -Wall -Wextra

shell: shell.c
	$(CC) $(CC_FLAGS) -o shell shell.c 

clean:
	@-rm -f shell *.o

.PHONY: clean
