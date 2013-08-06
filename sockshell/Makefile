CC = gcc
CFLAGS = -Wall -Wextra -lpthread

sockshell.o: sockshell.c
	$(CC) $(CFLAGS) -c $?

demo: sockshell.o demo.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm *.o demo
