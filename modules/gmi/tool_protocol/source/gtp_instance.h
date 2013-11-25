#ifndef __GMI_GTP_INSTANCE_H__
#define __GMI_GTP_INSTANCE_H__

#include "gtp_packet.h"
#include "gtp_list.h"
#include "gtp_schedule.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define GtpAddRef(ins)  do { (ins)->s_Reference.s_RefCount ++; } while (0)
#define GtpRelease(ins) \
    do { \
        if ((-- (ins)->s_Reference.s_RefCount) == 0) { \
            (ins)->s_Reference.s_DestroyFunc(ins); \
        } \
    } while (0)

#define GtpReferenceInit(ins, func) \
    do { \
        (ins)->s_Reference.s_RefCount = 1; \
        (ins)->s_Reference.s_DestroyFunc = (func); \
    } while (0)

typedef void_t (* GtpDestroyFunc)(void_t * Handle);

typedef struct tagGtpReference
{
    uint32_t       s_RefCount;
    GtpDestroyFunc s_DestroyFunc;
} GtpReference;

typedef struct tagGtpInstance
{
    // Reference
    GtpReference          s_Reference;

    // Pcap session handle
    PcapSessionHandle     s_PcapHandle;

    // Instance is used in server or client
    boolean_t             s_IsServer;

    // Session Id
    uint32_t              s_SessionId;

    // Callback function for sent data buffer
    CallbackSent          s_CbSent;
    void_t              * s_CbSentData;

    // Callback function for received data buffer
    CallbackRecv          s_CbRecv;
    void_t              * s_CbRecvData;

    // Callback function for transaction kill
    CallbackTransKilled   s_CbTransKilled;
    void_t              * s_CbTransKilledData;

    // Receive data buffer
    uint8_t               s_RecvBuf[GTP_MAX_PACKET_LENGTH];

    // Transaction count, used to allocate transaction id
    uint16_t              s_TransIdCount;

    // Transaction list
    GtpList               s_TransList;

    // Schedule task list
    GtpList               s_ScheduleTaskList;
} GtpInstance;

#ifdef __cplusplus
}
#endif


#endif // __GMI_GTP_INSTANCE_H__
