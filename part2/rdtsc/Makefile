CC = gcc

all: rdtsc_test.o 
	$(CC) -o rdtsc_exe $?

r_test.o: rdtsc_test.c
	$(CC) -c $?

clean:
	rm -f *.o rdtsc_exe
