
#include <libev/ev.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

static ev_timer st_EvtTimer;
static int st_EvtInserted=0;
static ev_timer st_ExpTimer;
static int st_ExpInserted=0;

static int st_RunLoop=1;
#define DEBUG_INFO(...) do{fprintf(stderr,"%s:%d[%ld]\t",__FILE__,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)
#define ERROR_INFO(...) do{fprintf(stderr,"%s:%d[%ld]\t",__FILE__,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)



void SigHandler(int signo)
{
	if (signo == SIGINT ||
		signo == SIGTERM)
	{
		st_RunLoop = 0;
	}
}

static void EvtTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg);
static void ExpTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg);


void StartEvtTimer()
{
    assert(st_EvtInserted == 0);
    ev_timer_init(&(st_EvtTimer),EvtTimerCallBack,3.0,0.0,NULL);
    ev_timer_start(EV_DEFAULT,&(st_EvtTimer));
    st_EvtInserted = 1;
}

void StopEvtTimer()
{
    if(st_EvtInserted)
    {
        ev_timer_stop(EV_DEFAULT,&(st_EvtTimer));
    }
    st_EvtInserted = 0;
    return ;
}

void StartExpTimer()
{
    assert(st_ExpInserted == 0);
    ev_timer_init(&(st_ExpTimer),ExpTimerCallBack,1.0,0.0,NULL);
    ev_timer_start(EV_DEFAULT,&(st_ExpTimer));
    st_ExpInserted = 1;
}

void StopExpTimer()
{
    if(st_ExpInserted)
    {
        ev_timer_stop(EV_DEFAULT,&(st_ExpTimer));
    }
    st_ExpInserted = 0;
    return ;

}


static void EvtTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg)
{
    DEBUG_INFO("\n");
    StopEvtTimer();
    StartEvtTimer();
    DEBUG_INFO("\n");
}


static void ExpTimerCallBack(EV_P_ ev_timer *w, int revents,void* arg)
{
    if(st_RunLoop == 0)
    {
        ev_break(EV_DEFAULT,EVBREAK_ONE);
		StopExpTimer();
		StopEvtTimer();
		return ;
    }
    DEBUG_INFO("\n");
    sleep(5);
    DEBUG_INFO("\n");
    StopExpTimer();
    StartExpTimer();
    StopEvtTimer();
    StartEvtTimer();
}


int main(int argc,char* argv[])
{

	signal(SIGINT,SigHandler);
	signal(SIGTERM,SigHandler);
	
    StartEvtTimer();
    StartExpTimer();

    ev_run(EV_DEFAULT,0);

    return 0;
}


