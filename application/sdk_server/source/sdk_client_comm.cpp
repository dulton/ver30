
#include <sdk_client_comm.h>

sdk_client_comm_t* AllocateComm(int datasize)
{
    sdk_client_comm_t* pComm=NULL;
	if (datasize > (int)sizeof(pComm->m_Data))
	{
		return NULL;
	}
    pComm = (sdk_client_comm_t*)malloc(sizeof(*pComm));
    if(pComm)
    {
        memset(pComm,0,sizeof(*pComm) - sizeof(pComm->m_Data));
		pComm->m_DataLen = datasize;
    }
    return pComm;
}

void FreeComm(sdk_client_comm_t * & pComm)
{
    if(pComm)
    {
        free(pComm);
    }
    pComm = NULL;
    return ;
}


