#ifndef _GMI_COMMON_H_
#define _GMI_COMMON_H_

#include "gmi_system_headers.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "gmi_includes.h"

#define CREATE_LOCK(lock) {pthread_mutex_init(lock,NULL);}
#define LOCK(lock) {pthread_mutex_lock(lock);}
#define UNLOCK(lock) {pthread_mutex_unlock(lock);}
#define RELEASE_LOCK(lock) {pthread_mutex_destroy(lock);}

//Max task number
#define MAX_TASK_NUM           0xFFFFFFFFUL
//Max task priority
#define MAX_TASK_PRIORITY      0x000000FFUL
//Min task priority
#define MIN_TASK_PRIORITY      0x00000000UL
//default task priority
#define DEFAULT_TASK_PRIORITY  0x00000064UL
//default task stack size 16K
#define MIN_TASK_STACK_SIZE    0x00004000UL


#define SAFE_DELETE(x) {if(x) {free(x);x=NULL;}}

//Task function
    typedef long_t (*fTaskEntryPoint)(long_t, long_t, long_t, long_t, long_t, long_t, long_t);

//Task Struct
    typedef struct _Tagtask
    {
        fTaskEntryPoint m_fun;     //pointer to task function
        long_t            m_param[7]; //task parameter
    } TASK,*PTASK;

    /*==============================================================
    name				:	GMI_TimeDelay
    function			:  System delay function
    algorithm implementation	:	no
    global variable			:	no
    parameter declaration		:     sec: delay second
                                                 usec: delay millisecond
    return				:    no
    ******************************************************************************/
    void GMI_TimeDelay(uint8_t sec, uint16_t usec);

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
    ******************************************************************************/
    long_t GMI_CreateTask(long_t lPriority, ulong_t lStackSize, fTaskEntryPoint fFun,
                          long_t lParam1, long_t lParam2, long_t lParam3, long_t lParam4,
                          long_t lParam5, long_t lParam6, long_t lParam7);

#ifdef __cplusplus
}
#endif

#endif /* DAEM_COMMON_H */

