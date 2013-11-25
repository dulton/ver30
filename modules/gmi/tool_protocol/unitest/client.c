#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <tool_protocol.h>

#include <debug.h>

static boolean_t l_Running = true;

inline static const char * Signal2String(int SigNo)
{
    switch (SigNo)
    {
#define CASE_SIGNAL(sig) case sig: return #sig

        CASE_SIGNAL(SIGINT);
        CASE_SIGNAL(SIGTERM);
        CASE_SIGNAL(SIGQUIT);

#undef CASE_SIGNAL
    }

    return "Unknown";
}

static void SignalStop(int SigNo)
{
    PRINT_LOG(INFO, "Receive signal %s", Signal2String(SigNo));
    if (l_Running)
    {
        PRINT_LOG(INFO, "Stopping process ...");
        l_Running = false;
    }
    else
    {
        PRINT_LOG(WARNING, "Process is already stopping ...");
    }
}

static void_t OnReceived(void_t * Ptr, GtpTransHandle Handle, const uint8_t * Buffer,
    uint32_t BufferLength, uint8_t Flags)
{
    PRINT_LOG(INFO, "OnReceived() is called");
    PRINT_LOG(INFO, "Buf[] = %s", Buffer);
}

static void_t OnTransKilled(void_t * Ptr, GtpTransHandle Handle)
{
    PRINT_LOG(INFO, "OnTransKilled() is called");
}

int main(int argc, const char * argv [])
{
    const char_t      * IfName     = "eth0";
    PcapSessionHandle   PcapHandle = PCAP_SESSION_INVALID_HANDLE;
    GtpHandle           GtpHandle  = GTP_INVALID_HANDLE;
    GtpTransHandle      GtpTHandle = GTP_INVALID_HANDLE;
    GMI_RESULT          RetVal     = GMI_SUCCESS;

    signal(SIGINT,  SignalStop);
    signal(SIGTERM, SignalStop);
    signal(SIGQUIT, SignalStop);

    if (argc > 1 && NULL == argv[1])
    {
        IfName = argv[1];
    }

    do
    {
        PcapHandle = PcapSessionOpen(IfName, 100);
        if (PCAP_SESSION_INVALID_HANDLE == PcapHandle)
        {
            PRINT_LOG(ERROR, "Can not open interface: %s", IfName);
            break;
        }

        GtpHandle = GtpCreateClientHandle(PcapHandle);
        if (GTP_INVALID_HANDLE == GtpHandle)
        {
            PRINT_LOG(ERROR, "Failed to create GTP handle");
            break;
        }

        GtpTHandle = GtpCreateBroadcastTransHandle(GtpHandle);
        if (GTP_INVALID_HANDLE == GtpTHandle)
        {
            PRINT_LOG(ERROR, "Failed to create broad cast transaction");
            break;
        }

        GtpSetRecvCallback(GtpHandle, OnReceived, (void *)GtpTHandle);
        GtpSetTransKilledCallback(GtpHandle, OnTransKilled, NULL);

        while (l_Running)
        {
            RetVal = GtpDoEvent(GtpHandle);
            if (RetVal != GMI_SUCCESS)
            {
                PRINT_LOG(ERROR, "Failed to do event");
                break;
            }

            usleep(10000);

            RetVal = GtpTransSendData(GtpTHandle, (uint8_t *)"0123456789", 11, 0);
            if (RetVal != GMI_SUCCESS)
            {
                PRINT_LOG(ERROR, "Failed to send data: 0123456789");
                break;
            }
        }

    } while (0);

    if (GTP_INVALID_HANDLE != GtpTHandle)
    {
        RetVal = GtpDestroyTransHandle(GtpTHandle);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to destroy transaction handle");
        }
    }

    if (GTP_INVALID_HANDLE != GtpHandle)
    {
        RetVal = GtpDestroyHandle(GtpHandle);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to destroy GTP handle");
        }
    }

    if (PCAP_SESSION_INVALID_HANDLE != PcapHandle)
    {
        RetVal = PcapSessionClose(PcapHandle);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to close handle");
            return 0;
        }
    }

    return 0;
}
