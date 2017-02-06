CC = gcc

target: timing_clkget_exe clock_gettime_exe

timing_clkget_exe: timing_gettime.o
	$(CC) -o timing_clkget_exe $?

clock_gettime_exe: clock_gettime.o
	$(CC) -o clock_gettime_exe $?

clock_gettime.o: clock_gettime.c
	$(CC) -c $?

timing_gettime.o: timing_gettime.c
	$(CC) -c $?

clean:
	rm -f *.o clock_gettime_exe timing_clkget_exe
