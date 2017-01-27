C = gcc

target: rdtsc_exe timing_rdtsc_exe

rdtsc_exe: rdtsc_test.o 
	$(CC) -o rdtsc_exe $?

timing_rdtsc_exe: timing_rdtsc.o
	$(CC) -o timing_rdtsc_exe $?

rdtsc_test.o: rdtsc_test.c
	$(CC) -c $?

timing_rdtsc.o: timing_rdtsc.c
	$(CC) -c $?

clean:
	rm -f *.o rdtsc_exe timing_rdtsc_exe
