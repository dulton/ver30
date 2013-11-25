#include "service.h"
#include "service_dispatch.h"

void_t IService::OnSent(GtpTransHandle Handle, GMI_RESULT Errno)
{
    PRINT_LOG(VERBOSE, "%s to send data packet", Errno == GMI_SUCCESS ? "Succeeded" : "Failed");

    // Dump variables
    DUMP_VARIABLE(Handle);
    DUMP_VARIABLE(Errno);

    // Unbind this transaction
    // Here do not need to unbind this transaction
    // Because GtpDestroyTransHandle() will trigger OnTransactionKilled()
    // Unbind(Handle);

    // Destroy this transaction
    GtpDestroyTransHandle(Handle);
}

void_t IService::OnRecv(GtpTransHandle Handle, const uint8_t * Buffer, uint32_t BufferLength, uint8_t Type)
{
    PRINT_LOG(VERBOSE, "Received data packet");

    // Dump variables
    DUMP_VARIABLE(Handle);
    DUMP_BUFFER(Buffer, BufferLength);
    DUMP_VARIABLE(Type);

    // Unbind this transaction
    Unbind(Handle);

    // We need to dispatch received data again
    ServiceDispatch::GetInstance().Dispatch(Handle, Buffer, BufferLength, Type);
}

void_t IService::OnTransactionKilled(GtpTransHandle Handle)
{
    PRINT_LOG(VERBOSE, "Transaction is killed");

    // Dump variables
    DUMP_VARIABLE(Handle);

    // Unbind this transaction
    Unbind(Handle);
}

