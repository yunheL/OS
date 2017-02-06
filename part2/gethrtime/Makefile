CC = gcc

all: gethrtime_test.o 
	$(CC) -o gethrtime_exe $?

gethrtime_test.o: gethrtime_test.c
	$(CC) -c $?

clean:
	rm -f *.o gethrtime_exe
