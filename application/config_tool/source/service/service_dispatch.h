#ifndef __SERVICE_DISPATCH_H__
#define __SERVICE_DISPATCH_H__

#include <string>
#include <map>

#include <tool_protocol.h>

#include "service.h"

class ServiceDispatch : public Instance<ServiceDispatch>
{
friend class Instance<ServiceDispatch>;

public:
    // Event procs for protocol layer
    static void_t OnSentProc(void_t * Data, GtpTransHandle Handle, GMI_RESULT Errno);
    static void_t OnRecvProc(void_t * Data, GtpTransHandle Handle, const uint8_t * Buffer, uint32_t BufferLength, uint8_t Type);
    static void_t OnTransactionKilledProc(void_t * Data, GtpTransHandle Handle);

    // Register/Unregister parser and service for received data
    GMI_RESULT Register(IParser * Parser, IService * Svr);
    GMI_RESULT Unregister(IParser * Parser);

    // Dispatch events
    void_t Dispatch(GtpTransHandle Handle, GMI_RESULT Errno);
    void_t Dispatch(GtpTransHandle Handle, const uint8_t * Buffer, uint32_t BufferLength, uint8_t Type);
    void_t Dispatch(GtpTransHandle Handle);

private:
    ServiceDispatch();
    ~ServiceDispatch();

    std::map<IParser *, IService *> m_ServiceTable;
};

#endif // __SERVICE_DISPATCH_H__

