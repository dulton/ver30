/******************************************************************************
modules		:    Daemon
name		:    common.c
function	:    System task common file
author		:    minchao.wang
version		:    1.0.0.0.0
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	1/29/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
#include "gmi_common.h"
#include "gmi_system.h"
#include "gmi_struct.h"
#include "gmi_config.h"
#include "gmi_daemon_thread.h"
#include "gmi_update.h"


/*==============================================================
name				:	GMI_StartTask
function			:  Start Up task function
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     p: Task function pointer
return				:    no
******************************************************************************/
static void *GMI_StartTask(void *p);

/*==============================================================
name				:	GMI_TaskPriority
function			:  Base on  the specify priority,Calculate task dynamic priority
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     lBase: the specify priority.Base on this value ,calculate to dynamic priority
return				:    long_t : the suitable priority
******************************************************************************/
static long_t GMI_TaskPriority(ulong_t lBase);

/*==============================================================
name				:	GMI_TimeDelay
function			:  System delay function
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     sec: delay second
                                             usec: delay millisecond
return				:    no
---------------------------------------------------------------
modification	:
   date		version		 author 	modification
   1/29/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
void GMI_TimeDelay(uint8_t sec, uint16_t usec)
{

    struct timeval delay;
    delay.tv_sec = sec;
    delay.tv_usec = usec;

    select(0, NULL, NULL, NULL, &delay);

}

/*==============================================================
name				:	GMI_StartTask
function			:  Start Up task function
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     p: Task function pointer
return				:    no
---------------------------------------------------------------
modification	:
   date		version		 author 	modification
   1/29/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
static void *GMI_StartTask(void *p)
{
    if (!p)
    {
        return NULL;
    }

    TASK task;
    task = *((TASK*)p);
    SAFE_DELETE(p);

    (*(task.m_fun))(task.m_param[0], task.m_param[1], task.m_param[2],
                    task.m_param[3], task.m_param[4], task.m_param[5],task.m_param[6]);
    return NULL;
}

/*==============================================================
name				:	GMI_TaskPriority
function			:  Base on  the specify priority,Calculate task dynamic priority
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     lBase: the specify priority.Base on this value ,calculate to dynamic priority
return				:    long_t : the suitable priority
---------------------------------------------------------------
modification	:
   date		version		 author 	modification
   1/29/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
static long_t GMI_TaskPriority(ulong_t lBase)
{

    long_t lMin = sched_get_priority_min(SCHED_FIFO);
    long_t lMax = sched_get_priority_max(SCHED_FIFO);
    long_t lReturn = lBase;
    if ( lBase>MAX_TASK_PRIORITY || lBase<MIN_TASK_PRIORITY )
    {
        lReturn = DEFAULT_TASK_PRIORITY;
    }

    lReturn = (MAX_TASK_PRIORITY-lBase)%(lMax-1);
    if ( lReturn<lMin )
    {
        lReturn = lMin;
    }
    return lReturn;

}

/*==============================================================
name				:	GMI_CreateTask
function			:  Create task interface, Create new task
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     IPriority: Task base dynamic priority
                                              IStackSize: stack size
                                              fFun: task function
                                              IParam1~IParam7: task parameter
return				:   SUCCESS: GMI_SUCCESS 0
                                      FAIL:  GMI_FAIL -1
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	1/29/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
long_t GMI_CreateTask(long_t lPriority, ulong_t lStackSize, fTaskEntryPoint fFun,
                      long_t lParam1, long_t lParam2, long_t lParam3, long_t lParam4,
                      long_t lParam5, long_t lParam6, long_t lParam7)
{
    if (!fFun)
    {
        return GMI_FAIL;
    }
    pthread_t pid = -1;
    long_t lReturn = GMI_FAIL;
    struct sched_param thread_sched_param;
    pthread_attr_t thread_attr;
    TASK *pNewTask = NULL;

    pthread_attr_init(&thread_attr);
    if (lStackSize < MIN_TASK_STACK_SIZE)
    {
        lStackSize = MIN_TASK_STACK_SIZE;
    }
    pthread_attr_setstacksize(&thread_attr, lStackSize);

    pthread_attr_getschedparam(&thread_attr, &thread_sched_param);
    pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
    thread_sched_param.sched_priority = GMI_TaskPriority(lPriority);
    pthread_attr_setschedparam(&thread_attr, &thread_sched_param);

    pNewTask = (TASK *)malloc(sizeof(TASK));
    if (pNewTask)
    {
        bzero(pNewTask, sizeof(TASK));
        pNewTask->m_fun = fFun;
        pNewTask->m_param[0] = lParam1;
        pNewTask->m_param[1] = lParam2;
        pNewTask->m_param[2] = lParam3;
        pNewTask->m_param[3] = lParam4;
        pNewTask->m_param[4] = lParam5;
        pNewTask->m_param[5] = lParam6;
        pNewTask->m_param[6] = lParam7;
        lReturn =  pthread_create(&pid, &thread_attr, GMI_StartTask, (void*)pNewTask);
        pthread_attr_destroy(&thread_attr);
        return lReturn!=0?(GMI_FAIL):(GMI_SUCCESS);
    }
    return GMI_FAIL;
} /*  end of GMI_CreateTask  */



