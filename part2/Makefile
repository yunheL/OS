<<<<<<< HEAD
CC = gcc

all: r_test.o gettimedayday.o
	$(CC) -o test $?

r_test.o: r_test.c
	$(CC) -c $?

r_test.o: gettimeofday.c
	$(CC) -c $?

clean:
	rm -f *.o test
=======
pinCPU: pinCPU.c pinCPU.o
	gcc -Wall -o pinCPU pinCPU.c

clean:
	rm pinCPU.o pinCPU
>>>>>>> 0faf1d68fbe74164d155f11aaa7a109ad95972ef
