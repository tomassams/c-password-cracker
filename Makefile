TARGET = cracker
CFLAGS = -Wall -Wextra -lcrypt -pthread -std=c11
CC = gcc

$(TARGET): brute_force.o dictionary.o helper.o main.o
	$(CC) main.o helper.o dictionary.o brute_force.o $(CFLAGS) -o $(TARGET)

main.o: main.c
	$(CC) $(CFLAGS) -c -o main.o main.c

helper.o: helper.c
	$(CC) $(CFLAGS) -c -o helper.o helper.c

dictionary.o: dictionary.c
	$(CC) $(CFLAGS) -c -o dictionary.o dictionary.c

brute_force.o: brute_force.c
	$(CC) $(CFLAGS) -c -o brute_force.o brute_force.c

clean: 
	rm *.o $(TARGET)

remake: clean $(TARGET)
