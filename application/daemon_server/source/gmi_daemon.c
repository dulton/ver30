/******************************************************************************
modules		:    Daemon
name		:    Daemon.c
function	:    main function
author		:    minchao.wang
version		:    1.0.0.0.0
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	1/28/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
#include "gmi_includes.h"
#include "gmi_hardware_monitor.h"
#include "gmi_system.h"
#include "gmi_struct.h"
#include "gmi_config.h"
#include "gmi_daemon_thread.h"
#include "gmi_update.h"
#include "gmi_system_headers.h"

void sigHandler(int signum)
{

    if (signum == SIGINT)
    {
        printf("receive SIGINT (Ctrl-C) signal\n");
        exit(0);
    }
    else if (signum == SIGIO)
    {
        printf("receive SIGIO signal\n");
    }
    else if (signum == SIGPIPE)
    {
        printf("receive SIGPIPE signal\n");
    }
    else if (signum == SIGHUP)
    {
        printf("receive SIGHUP signal\n");
        exit(0);
    }
    else if (signum == SIGQUIT)
    {
        printf("receive SIGQUIT signal\n");
        exit(0);
    }
    else if (signum == SIGSEGV)
    {
        sleep(1);
    }
    else
    {
        printf("receive unknown signal. signal = %d\n", signum);
    }
    return;
}

int main(int32_t argc, char_t **argv)
{
    int iSignal;
    struct sigaction sa;
    sigset_t newMask;
    sigset_t oldMask;
    pid_t    iChildPID;

    iChildPID = fork();
    if (iChildPID < 0)
    {
        perror("fork()");
        exit(1);
    }
    else if (iChildPID > 0)
    {
        exit(0);
    }
    printf("GMI:[%s]new process id:%d\n", __func__, getpid());
    setsid();
    umask(0);

    sa.sa_handler = sigHandler;
    sigfillset(&sa.sa_mask);
    sa.sa_flags = SA_NOMASK;
    sigemptyset(&newMask);
    for (iSignal=1; iSignal<=_NSIG; ++iSignal)
    {
        if ((iSignal == SIGIO)
                || (iSignal == SIGPOLL)
                || (iSignal == SIGINT)
                || (iSignal == SIGQUIT)
                || (iSignal == SIGHUP)
                || (iSignal == SIGPIPE)
                || (iSignal == SIGSEGV)
           )
        {
            sigaction(iSignal, &sa, NULL);
        }
        else
        {
            sigaddset(&newMask, iSignal);
        }
    }
    sigprocmask(SIG_BLOCK, &newMask, &oldMask);

    GMI_SystemInitial();

    while (1)
    {
        pause();
    }

    return EXIT_SUCCESS;
}

