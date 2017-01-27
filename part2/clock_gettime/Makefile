CC = gcc

all: clock_gettime.o 
	$(CC) -o clock_gettime_exe $?

clock_gettime.o: clock_gettime.c
	$(CC) -c $?

clean:
	rm -f *.o clock_gettime_exe
