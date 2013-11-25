
#include <stdlib.h>
#include <stdio.h>

class AClass
{
public:
    AClass() {};
    ~AClass() {};
};

class HelloHeap
{
public:
    HelloHeap()
    {
        m_pACls = new AClass();
    };
    ~HelloHeap()
    {
        if(m_pACls)
        {
            delete m_pACls;
        }
        m_pACls = NULL;
    };
private:
    AClass *m_pACls;
};


int main(int argc,char* argv[])
{
    int i;
    HelloHeap* pHello=NULL;

    for(i=0; i<10; i++)
    {
        pHello = new HelloHeap();
        if(pHello)
        {
            if((i%2)==0)
            {
                delete pHello;
            }
        }

        pHello = NULL;
    }


    return 0;
}

