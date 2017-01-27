Last login: Tue Jan 24 14:28:08 on ttys000
-bash: /usr/local/cuda/lib: is a directory
Yunhes-MacBook-Pro:~ yunhe$ ls
Anaconda2-4.2.0-MacOSX-x86_64.sh	Library
AndroidStudioProjects			Movies
Applications				Music
Desktop					Pictures
Documents				Public
Downloads				anaconda2
Dropbox					target
Google Drive				tensorflow
Yunhes-MacBook-Pro:~ yunhe$ cd Documents/
Yunhes-MacBook-Pro:Documents yunhe$ ls
Career					files
G1A					git
G1B					git_backup
GradApp					grad_school
LGD					legal_doc
MATLAB					pic
Travel					receipt
Work					workspace
courses					~$rsonal Statement of Yunhe Liu.docx
Yunhes-MacBook-Pro:Documents yunhe$ cd G1B
Yunhes-MacBook-Pro:G1B yunhe$ ls
736		OS_text		coursera_data
760		TA		plans
Yunhes-MacBook-Pro:G1B yunhe$ cd 736
Yunhes-MacBook-Pro:736 yunhe$ ls
Paper		mini_pro1
Yunhes-MacBook-Pro:736 yunhe$ cd mini_pro1/
Yunhes-MacBook-Pro:mini_pro1 yunhe$ ls
Makefile	rdtsc_test.cpp
Yunhes-MacBook-Pro:mini_pro1 yunhe$ vim Makefile 
Yunhes-MacBook-Pro:mini_pro1 yunhe$ ls
Makefile	rdtsc_test.cpp
Yunhes-MacBook-Pro:mini_pro1 yunhe$ vim rdtsc_test.cpp 

// rdtsc.cpp  
// processor: x86, x64  
#include <stdio.h>
#include <intrin.h>

#pragma intrinsic(__rdtsc)  

int main()
{
    unsigned __int64 i;
    unsigned __int64 j;
    unsigned __int64 diff;

    i = __rdtsc();
    printf_s("%I64d ticks\n", i);

    sleep(10);

    j = __rdtsc();
    printf_s("%I64d ticks\n", j);

    diff = j - i;
    printf_s("%I64d ticks\n", diff);
}

