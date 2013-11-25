#ifndef __GMI_TOOL_PROTOCOL_H__
#define __GMI_TOOL_PROTOCOL_H__

#include <gmi_type_definitions.h>
#include <gmi_errors.h>

#include <pcap_session.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Define GMI Tool Protocol invalid handle
#define GTP_INVALID_HANDLE   (void_t *)0

// Define send flags
#define GTP_DATA_TYPE_BINARY     0
#define GTP_DATA_TYPE_UTF8_XML   1
#define GTP_DATA_TYPE_USER1      2
#define GTP_DATA_TYPE_USER2      3

// Define GMI Tool Protocol handle
typedef void_t * GtpHandle;

// Define GMI Tool Protocol transaction handle 
typedef void_t * GtpTransHandle;

// Define data sent callback
typedef void_t (* CallbackSent)(void_t * Ptr, GtpTransHandle Handle, GMI_RESULT Errno);

// Define data receive callback
typedef void_t (* CallbackRecv)(void_t * Ptr, GtpTransHandle Handle,
    const uint8_t * Buffer, uint32_t BufferLength, uint8_t Type);

// Define transaction killed callback
typedef void_t (* CallbackTransKilled)(void_t * Ptr, GtpTransHandle Handle);

// Define delay task callback
typedef void_t (* CbOnSchedule)(void_t * Data);

#define GtpCreateServerHandle(Hnd) GtpCreateHandle(Hnd, true)
#define GtpCreateClientHandle(Hnd) GtpCreateHandle(Hnd, false)

/*
 * Handle       : Handle of pcap session instance.
 * IsServer     : true  - Create handle used in server
 *                false - Create handle used in client
 * Return Value : GTP_INVALID_HANDLE - failed to create handle,
 *                Other values       - handle of GTP instance.
 */
GtpHandle GtpCreateHandle(PcapSessionHandle Handle, boolean_t IsServer);

/*
 * Handle       : Handle of GTP instance.
 * Return Value : GMI_SUCCESS - success,
 *                Other Value - failed.
 */
GMI_RESULT GtpDestroyHandle(GtpHandle Handle);

/*
 * Handle       : Handle of GTP instance.
 * CbFunc       : Function pointer for sent callback.
 * Data         : User date.
 */
void_t GtpSetSentCallback(GtpHandle Handle, CallbackSent CbFunc, void_t * Data);

/*
 * Handle       : Handle of GTP instance.
 * CbFunc       : Function pointer for receive callback.
 * Data         : User date.
 */
void_t GtpSetRecvCallback(GtpHandle Handle, CallbackRecv CbFunc, void_t * Data);

/*
 * Handle       : Handle of GTP instance.
 * CbFunc       : Function pointer for transaction killed callback.
 * Data         : User date.
 */
void_t GtpSetTransKilledCallback(GtpHandle Handle, CallbackTransKilled CbFunc,
    void_t * Data);

/*
 * Handle       : Handle of GTP instance.
 * CbFunc       : Function pointer for delay task callback.
 * Data         : User date.
 * Sec          : Delay time in seconds.
 * USec         : Delay time in microseconds.
 */
GMI_RESULT GtpAddDelayTask(GtpHandle Handle, CbOnSchedule CbFunc, void_t * Data,
    uint32_t Sec, uint32_t USec);

/*
 * Handle       : Handle of GTP instance.
 * CbFunc       : Function pointer for delay task callback.
 * Data         : User date.
 * Sec          : Delay time in seconds.
 * USec         : Delay time in microseconds.
 */
GMI_RESULT GtpUpdateDelayTask(GtpHandle Handle, CbOnSchedule CbFunc, void_t * Data,
    uint32_t Sec, uint32_t USec);

/*
 * Handle       : Handle of GTP instance.
 * CbFunc       : Function pointer for delay task callback.
 * Data         : User date.
 */
GMI_RESULT GtpCancelDelayTask(GtpHandle Handle, CbOnSchedule CbFunc, void_t * Data);

/*
 * Handle       : Handle of GTP instance.
 * Return Value : GMI_SUCCESS - success,
 *                Other Value - failed.
 */
GMI_RESULT GtpDoEvent(GtpHandle Handle);

/*
 * Handle       : Handle of GTP instance.
 * MacAddr      : Mac address, destination of this transaction
 * Return Value : GTP_INVALID_HANDLE - failed to create handle,
 *                Other values       - handle of GTP transaction instance.
 */
GtpTransHandle GtpCreateTransHandle(GtpHandle Handle,
    const uint8_t MacAddr[MAC_ADDRESS_LENGTH]);

/*
 * Handle       : Handle of GTP instance.
 * Return Value : GTP_INVALID_HANDLE - failed to create handle,
 *                Other values       - handle of GTP transaction instance.
 */
GtpTransHandle GtpCreateBroadcastTransHandle(GtpHandle Handle);

/*
 * Handle       : Handle of GTP transaction instance.
 * Return Value : GMI_SUCCESS - success,
 *                Other Value - failed.
 */
GMI_RESULT GtpDestroyTransHandle(GtpTransHandle Handle);

/*
 * Handle       : Handle of GTP transaction instance.
 * Return Value : GTP_INVALID_HANDLE - failed to get handle of GTP instance,
 *                Other values       - handle of GTP instance.
 */
GtpHandle GtpTransGetGtpHandle(GtpTransHandle Handle);

/*
 * Handle       : Handle of GTP transaction instance.
 * MacAddr      : [out] MAC address of this GTP transaction.
 * Return Value : GMI_SUCCESS - success,
 *                Other Value - failed.
 */
GMI_RESULT GtpTransGetMacAddress(GtpTransHandle Handle,
    uint8_t MacAddr[MAC_ADDRESS_LENGTH]);

/*
 * Handle       : Handle of GTP transaction instance.
 * Buffer       : Point to the data buffer need to be sent.
 * BufferLength : Length of the data buffer.
 * Type         : Data type.
 * Return Value : GMI_SUCCESS - success,
 *                Other Value - failed.
 */
GMI_RESULT GtpTransSendData(GtpTransHandle Handle, const uint8_t * Buffer,
    uint32_t BufferLength, uint8_t Type);

/*
 * Handle       : Handle of GTP transaction instance.
 * Ptr          : Reference instance.
 */
void_t GtpTransSetReferenceInstance(GtpTransHandle Handle, void_t * Ptr);

/*
 * Handle       : Handle of GTP transaction instance.
 * Return Value : Reference instance.
 */
void_t * GtpTransGetReferenceInstance(GtpTransHandle Handle);

#ifdef __cplusplus
}
#endif

#endif // __GMI_TOOL_PROTOCOL_H__
