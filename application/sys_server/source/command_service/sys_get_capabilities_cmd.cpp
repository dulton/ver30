#include "sys_get_capabilities_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"
#include "application_packet_header.h"

SysGetCapabilitiesCommandExecutor::SysGetCapabilitiesCommandExecutor(void)
    : SysBaseCommandExecutor(SYSCODE_GET_CAPABILITIES_REQ, CT_ShortTask )
{
}


SysGetCapabilitiesCommandExecutor::~SysGetCapabilitiesCommandExecutor(void)
{
}


GMI_RESULT  SysGetCapabilitiesCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysGetCapabilitiesCommandExecutor> GetCapabilitiesCommandExecutor( BaseMemoryManager::Instance().New<SysGetCapabilitiesCommandExecutor>() );
    if (NULL == GetCapabilitiesCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    GetCapabilitiesCommandExecutor->m_Session = Packet->GetSession();
    GetCapabilitiesCommandExecutor->m_Reply = Packet->Clone();
    GMI_RESULT Result = GetCapabilitiesCommandExecutor->SetParameter(m_SystemServiceManager, m_Argument, m_ArgumentSize);
    if (FAILED(Result))
    {
        return Result;
    }

    CommandExecutor = GetCapabilitiesCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysGetCapabilitiesCommandExecutor::Execute()
{
#define           MESSAGE_LENGTH  16384
    int32_t           MessageCode     = RETCODE_OK;
    GMI_RESULT        Result          = GMI_SUCCESS;
    SystemPacket     *CommandPacket   = (SystemPacket*)m_Reply.GetPtr();
    uint8_t          *PayloadBuff     = CommandPacket->GetPacketPayloadBuffer();
    SysPkgAttrHeader *SysPkgAttrHdPtr = (SysPkgAttrHeader*)PayloadBuff;

    int32_t            CapabilityType;
    int32_t            GetCapbilityLen = 0;
    SysPkgXml          SysCapabilities;
    ReferrencePtr<char_t, DefaultObjectsDeleter> CapabilitiesMessage(BaseMemoryManager::Instance().News<char_t>(MESSAGE_LENGTH));
    if (NULL == CapabilitiesMessage.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    do
    {
        if (1 == CommandPacket->GetAttrCount())
        {
            SysPkgAttrHdPtr->s_Type = NETWORK_TO_HOST_USHORT(SysPkgAttrHdPtr->s_Type);
            if (TYPE_INTVALUE != SysPkgAttrHdPtr->s_Type)
            {
                MessageCode = RETCODE_ERROR;
                break;
            }
            //int32_t CapabilityType = *(int32_t*)(PayloadBuff + sizeof(SysPkgAttrHeader));
            CapabilityType = NETWORK_TO_HOST_UINT(*(int32_t*)(PayloadBuff + sizeof(SysPkgAttrHeader)));
        }
        else
        {
            CapabilityType = SYS_CAPABILITY_CATEGORY_ALL;
        }

        memset(&SysCapabilities, 0, sizeof(SysPkgXml));

        Result = m_SystemServiceManager->GetCapabilities(CapabilityType, MESSAGE_LENGTH, CapabilitiesMessage.GetPtr(), &SysCapabilities);
        if (FAILED(Result))
        {
            MessageCode = RETCODE_ERROR;
            break;
        }
        //memset(&SysCapabilities, 0, sizeof(SysPkgXml));
        //SYS_ERROR("\n");

        GetCapbilityLen = SysCapabilities.s_ContentLength;
        SysCapabilities.s_ContentLength      = HOST_TO_NETWORK_UINT(SysCapabilities.s_ContentLength);
        SysCapabilities.s_Encrypt            = HOST_TO_NETWORK_UINT(SysCapabilities.s_Encrypt);
    }
    while (0);

    // Reply
    uint16_t PacketHeaderSize = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1;
    uint16_t PayloadTotalSize;
    uint16_t AppPacketSize;

    if (MessageCode == RETCODE_OK)
    {
        PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgXml) + GetCapbilityLen;
    }
    else
    {
        PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgMessageCode);
    }
    PayloadTotalSize = PacketAttrHdSize1 + sizeof(SdkPkgTransferProtocolKey);
    AppPacketSize    = PacketHeaderSize  + PacketAttrHdSize1;
    ReferrencePtr<SystemPacket> Reply(BaseMemoryManager::Instance().New<SystemPacket>());
    if (NULL == Reply.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    Result = Reply->AllocatePacketBuffer(0, PayloadTotalSize);
    if (FAILED(Result))
    {
        return Result;
    }

    //fill packet header
    Result = SystemPacket::FillPacketHeader(
                 Reply->GetPacketHeaderBuffer(),
                 CommandPacket->GetMessageTag(),
                 CommandPacket->GetVersion(),
                 SYSCODE_GET_CAPABILITIES_RSP,
                 1,
                 AppPacketSize,
                 CommandPacket->GetSessionId(),
                 CommandPacket->GetSequenceNumber()
             );
    if (FAILED(Result))
    {
        SYS_ERROR("FillPacketHeader fail\n");
        return Result;
    }

    //fill packet body
    uint8_t *PayloadBuffer = Reply->GetPacketPayloadBuffer();
    uint16_t Offset = 0;
    SysPkgAttrHeader *SysPkgAtrrHdPtr;

    if (MessageCode == RETCODE_OK)
    {
        SysPkgXml *SysCapabilitiesPtr;
        char_t    *MessagePtr;

        SysPkgAtrrHdPtr = (SysPkgAttrHeader*)PayloadBuffer;
        SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_XML);
        SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
        Offset                   += sizeof(SysPkgAttrHeader);
        SysCapabilitiesPtr        = (SysPkgXml*)(PayloadBuffer + Offset);
        memcpy((void_t*)SysCapabilitiesPtr, (void_t*)&SysCapabilities,  sizeof(SysPkgXml));
        Offset                   += sizeof(SysPkgXml);
        MessagePtr                = (char_t*)(PayloadBuffer + Offset);
        memcpy(MessagePtr, CapabilitiesMessage.GetPtr(), GetCapbilityLen);
        Offset                   += GetCapbilityLen;
    }
    else
    {
        SysPkgMessageCode *SysPkgMessageCodePtr;

        SysPkgAtrrHdPtr = (SysPkgAttrHeader*)PayloadBuffer;
        SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_MESSAGECODE);
        SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
        Offset                   += sizeof(SysPkgAttrHeader);
        SysPkgMessageCodePtr      = (SysPkgMessageCode*)(PayloadBuffer + Offset);
        SysPkgMessageCodePtr->s_MessageCode = HOST_TO_NETWORK_UINT(MessageCode);
        SysPkgMessageCodePtr->s_MessageLen  = 0;
        Offset                   += sizeof(SysPkgMessageCode);
        Offset                   += SysPkgMessageCodePtr->s_MessageLen;
    }
    //SYS_ERROR("line = %d\n", __LINE__);
    //fill packet tail
    Result = SystemPacket::FillPacketSdkTransferProtocolKey(
                 (PayloadBuffer + Offset),
                 CommandPacket->GetSdkTransferKeyTag(),
                 CommandPacket->GetSdkTransferProtocolSessionId(),
                 CommandPacket->GetSdkTransferProtocolSequenceNumber(),
                 CommandPacket->GetSdkTransferProtocolAuthValue()
             );
    if (FAILED(Result))
    {
        SYS_ERROR("FillPacketSdkTransferProtocolKey fail\n");
        return Result;
    }
    m_Reply = Reply;

    return GMI_SUCCESS;
}


GMI_RESULT  SysGetCapabilitiesCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}






