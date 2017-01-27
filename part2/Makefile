CC = gcc

all: r_test.o gettimedayday.o
	$(CC) -o test $?

r_test.o: r_test.c
	$(CC) -c $?

r_test.o: gettimeofday.c
	$(CC) -c $?

clean:
	rm -f *.o test
