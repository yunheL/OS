#!/bin/bash

a=1
gcc bplusNew_S10k.c
while [ $a -lt 11 ]
do
	./a.out 10000
        #echo $a
        a=`expr $a + 1`
	
done
a=1
gcc bplusNew_S100k.c
while [ $a -lt 11 ]
do
	./a.out 100000
        #echo $a
        a=`expr $a + 1`
	
done
a=1
gcc bplusNew_SingleIns.c
while [ $a -lt 11 ]
do
	./a.out 1
        #echo $a
        a=`expr $a + 1`
done
a=1
gcc bplusNew_SingleSplit.c
while [ $a -lt 11 ]
do
	./a.out 1
        #echo $a
        a=`expr $a + 1`
	
done
