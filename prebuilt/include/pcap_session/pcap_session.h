#ifndef __GMI_PCAP_SESSION_H__
#define __GMI_PCAP_SESSION_H__

#include <gmi_type_definitions.h>
#include <gmi_errors.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Define MAC address length
#define MAC_ADDRESS_LENGTH          6

// Define max buffer length to transport.
#define PCAP_SESSION_MAX_BUF_LENGTH 1498

// Define invalid value of pcap session handle
#define PCAP_SESSION_INVALID_HANDLE (void_t *)0

// Define pcap session handle
typedef void * PcapSessionHandle;

/*
 * IfName       : Interface name, MUST NOT be NULL.
 * RecvTimedOut : Timed out time for receive function, unit is millisecond.
 * Return Value : PCAP_SESSION_INVALID_HANDLE - failed to open interface,
 *                Other values                - handle of pcap session instance.
 */
PcapSessionHandle PcapSessionOpen(const char_t * IfName, uint32_t RecvTimedOut);

/*
 * Handle       : Handle of pcap session instance.
 * Return Value : GMI_SUCCESS - success,
 *                Other Value - failed.
 */
GMI_RESULT PcapSessionClose(PcapSessionHandle Handle);

/*
 * Handle       : Handle of pcap session instance.
 * Buffer       : Point to the data buffer need to be broadcast.
 * BufferLength : Length of the data buffer, max is PCAP_SESSION_MAX_BUF_LENGTH.
 * Return Value : GMI_SUCCESS - success,
 *                Other Value - failed.
 */
GMI_RESULT PcapSessionBroadcast(PcapSessionHandle Handle, const uint8_t * Buffer,
    uint32_t BufferLength);

/*
 * Handle       : Handle of pcap session instance.
 * Buffer       : Point to the data buffer need to be sent.
 * BufferLength : Length of the data buffer, max is PCAP_SESSION_MAX_BUF_LENGTH.
 * MacAddr      : Destination of the data buffer.
 * Return Value : GMI_SUCCESS - success,
 *                Other Value - failed.
 */
GMI_RESULT PcapSessionSendTo(PcapSessionHandle Handle, const uint8_t * Buffer,
    uint32_t BufferLength, const uint8_t MacAddr[MAC_ADDRESS_LENGTH]);

/*
 * Handle       : Handle of pcap session instance.
 * Buffer       : Point to the data buffer that can be filled.
 * BufferLenPtr : [in ] Length of the data buffer, max is PCAP_SESSION_MAX_BUF_LENGTH,
 *                [out] Bytes filled into the data buffer.
 * IsBroadcast  : [out] false : Single cast,
 *                      true : Broad cast.
 * MacAddr      : Source of the data stream.
 * Return Value : GMI_SUCCESS      - success,
 *                GMI_WAIT_TIMEOUT - receive timed out,
 *                Other Value      - failed.
 */
GMI_RESULT PcapSessionRecvFrom(PcapSessionHandle Handle, uint8_t * Buffer,
    uint32_t * BufferLenPtr, uint8_t MacAddr[MAC_ADDRESS_LENGTH], boolean_t * IsBroadcast);

#ifdef __cplusplus
}
#endif

#endif

