
#include <gmi_system_headers.h>

#define DEBUG_INFO(...) do{fprintf(stderr,"%s:%d\t",__FILE__,__LINE__);fprintf(stderr,__VA_ARGS__);}while(0)

static int st_RunLoop=1;
static int st_ShareCount=0;
void* ThreadA(void*arg)
{
    GMI_Mutex* pMutex=(GMI_Mutex*)arg;
    GMI_RESULT gmiret;

    while(st_RunLoop)
    {
        DEBUG_INFO("Enter Lock A\n");
        gmiret = pMutex->Lock( TIMEOUT_INFINITE );
        assert(gmiret == GMI_SUCCESS);
        st_ShareCount += 1;
        assert(st_ShareCount == 1);
        DEBUG_INFO("In LockA\n");
        sleep(2);
        assert(st_ShareCount == 1);
        st_ShareCount -= 1;

        pMutex->Unlock();
        DEBUG_INFO("Exit LockA\n");
    }
    return 0;
}

void* ThreadB(void* arg)
{
    GMI_Mutex* pMutex=(GMI_Mutex*)arg;
    GMI_RESULT gmiret;

    while(st_RunLoop)
    {
        DEBUG_INFO("Enter LockB\n ");
        gmiret = pMutex->Lock(TIMEOUT_INFINITE);
        assert(gmiret == GMI_SUCCESS);
        DEBUG_INFO("In LockB\n");
        st_ShareCount += 1;
        assert(st_ShareCount == 1);
        sleep(3);
        assert(st_ShareCount == 1);
        st_ShareCount -= 1;

        pMutex->Unlock();
        DEBUG_INFO("Exit LockB\n");
    }
    return 0;
}

int main(int argc,char* argv[])
{
    GMI_Mutex* pMutex=NULL;
    GMI_Thread *pThread[2] = {NULL,NULL};
    GMI_RESULT gmiret;
    pMutex = new GMI_Mutex();
    gmiret = pMutex->Create(NULL);
    assert(gmiret == GMI_SUCCESS);
    pThread[0] = new GMI_Thread();
    pThread[1] = new GMI_Thread();

    gmiret = pThread[0]->Create(NULL,0,ThreadA,pMutex);
    assert(gmiret == GMI_SUCCESS);
    gmiret = pThread[1]->Create(NULL,0,ThreadB,pMutex);
    assert(gmiret == GMI_SUCCESS);
    gmiret = pThread[0]->Start();
    assert(gmiret == GMI_SUCCESS);
    gmiret = pThread[1]->Start();
    assert(gmiret == GMI_SUCCESS);

    while(st_RunLoop)
    {
        sleep(100);
    }
    return 0;
}
