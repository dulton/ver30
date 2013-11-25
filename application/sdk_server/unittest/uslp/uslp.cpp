
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

int main(int argc,char* argv[])
{
    int millcount=1;

    if(argc < 2)
    {
        fprintf(stderr,"%s millcount usleep\n",argv[0]);
        exit(3);
    }

    millcount = atoi(argv[1]);
    if(millcount == 0)
    {
        fprintf(stderr,"millcount == 0\n");
        exit(3);
    }

    while(1)
    {
        usleep(millcount * 1000);
        pthread_yield();		
    }

    return 0;
}

