#include "sys_get_encode_config_cmd.h"
#include "log.h"
#include "system_packet.h"
#include "sys_env_types.h"


SysGetEncodeConfigCommandExecutor::SysGetEncodeConfigCommandExecutor()
    :  SysBaseCommandExecutor( SYSCODE_GET_ENCODECFG_REQ, CT_ShortTask)
{
}


SysGetEncodeConfigCommandExecutor::~SysGetEncodeConfigCommandExecutor()
{
}


GMI_RESULT SysGetEncodeConfigCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    SystemPacket *SysPacket = (SystemPacket*) Packet.GetPtr();

    if (GetCommandId() != SysPacket->GetMessageCode())
    {
        return GMI_FAIL;
    }

    SafePtr<SysGetEncodeConfigCommandExecutor> GetEncodeCfgCommandExecutor(BaseMemoryManager::Instance().New<SysGetEncodeConfigCommandExecutor>() );
    if (NULL == GetEncodeCfgCommandExecutor.GetPtr())
    {
        return GMI_OUT_OF_MEMORY;
    }

    GetEncodeCfgCommandExecutor->m_Session = Packet->GetSession();
    GetEncodeCfgCommandExecutor->m_Reply = Packet->Clone();
    GetEncodeCfgCommandExecutor->SetParameter( m_SystemServiceManager, m_Argument, m_ArgumentSize);

    CommandExecutor = GetEncodeCfgCommandExecutor;

    return GMI_SUCCESS;
}


GMI_RESULT  SysGetEncodeConfigCommandExecutor::Execute()
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    int32_t          MessageCode     = RETCODE_OK;
    int32_t          StreamNum       = 0;
    SystemPacket    *CommandPacket   = (SystemPacket*) m_Reply.GetPtr();

    GMI_RESULT Result = m_SystemServiceManager->SvrGetVideoStreamNum(&StreamNum);
    if (FAILED(Result))
    {
        SYS_ERROR("SvrGetVideoStreamNum fail\n");
        MessageCode = RETCODE_ERROR;
    }

    ReferrencePtr<SysPkgEncodeCfg, DefaultObjectsDeleter>SysEncodeCfgPtr(BaseMemoryManager::Instance().News<SysPkgEncodeCfg>(StreamNum));
    if (NULL == SysEncodeCfgPtr.GetPtr())
    {
        SYS_ERROR("SysEncodeCfgPtr news fail\n");
        return GMI_OUT_OF_MEMORY;
    }
    memset(SysEncodeCfgPtr.GetPtr(), 0, sizeof(SysPkgEncodeCfg)*StreamNum);

    Result = m_SystemServiceManager->SvrGetVideoEncodeSettings(0xff, SysEncodeCfgPtr.GetPtr());
    if (FAILED(Result))
    {
        SYS_ERROR("SvrGetVideoEncodeSettings fail\n");
        MessageCode = RETCODE_ERROR;
    }
    else
    {
        for (int32_t Id = 0; Id < StreamNum; Id++)
        {
            (SysEncodeCfgPtr.GetPtr())[Id].s_VideoId        = HOST_TO_NETWORK_UINT((SysEncodeCfgPtr.GetPtr())[Id].s_VideoId);
            (SysEncodeCfgPtr.GetPtr())[Id].s_StreamType     = HOST_TO_NETWORK_UINT((SysEncodeCfgPtr.GetPtr())[Id].s_StreamType);
            (SysEncodeCfgPtr.GetPtr())[Id].s_Compression    = HOST_TO_NETWORK_UINT((SysEncodeCfgPtr.GetPtr())[Id].s_Compression);
            (SysEncodeCfgPtr.GetPtr())[Id].s_PicWidth       = HOST_TO_NETWORK_UINT((SysEncodeCfgPtr.GetPtr())[Id].s_PicWidth);
            (SysEncodeCfgPtr.GetPtr())[Id].s_PicHeight      = HOST_TO_NETWORK_UINT((SysEncodeCfgPtr.GetPtr())[Id].s_PicHeight);
            (SysEncodeCfgPtr.GetPtr())[Id].s_BitrateCtrl    = HOST_TO_NETWORK_UINT((SysEncodeCfgPtr.GetPtr())[Id].s_BitrateCtrl);
            (SysEncodeCfgPtr.GetPtr())[Id].s_Quality        = HOST_TO_NETWORK_UINT((SysEncodeCfgPtr.GetPtr())[Id].s_Quality);
            (SysEncodeCfgPtr.GetPtr())[Id].s_FPS            = HOST_TO_NETWORK_UINT((SysEncodeCfgPtr.GetPtr())[Id].s_FPS);
            (SysEncodeCfgPtr.GetPtr())[Id].s_BitRateAverage = HOST_TO_NETWORK_UINT((SysEncodeCfgPtr.GetPtr())[Id].s_BitRateAverage);
            (SysEncodeCfgPtr.GetPtr())[Id].s_BitRateUp      = HOST_TO_NETWORK_UINT((SysEncodeCfgPtr.GetPtr())[Id].s_BitRateUp);
            (SysEncodeCfgPtr.GetPtr())[Id].s_BitRateDown    = HOST_TO_NETWORK_UINT((SysEncodeCfgPtr.GetPtr())[Id].s_BitRateDown);
            (SysEncodeCfgPtr.GetPtr())[Id].s_Gop            = HOST_TO_NETWORK_UINT((SysEncodeCfgPtr.GetPtr())[Id].s_Gop);
            (SysEncodeCfgPtr.GetPtr())[Id].s_Rotate         = HOST_TO_NETWORK_UINT((SysEncodeCfgPtr.GetPtr())[Id].s_Rotate);
            (SysEncodeCfgPtr.GetPtr())[Id].s_Flag           = HOST_TO_NETWORK_UINT((SysEncodeCfgPtr.GetPtr())[Id].s_Flag);
        }
    }

    // Reply
    uint16_t PacketHeaderSize = sizeof(SysPkgHeader);
    uint16_t PacketAttrHdSize1;
    uint16_t PayloadTotalSize;
    uint16_t AppPacketSize;
    uint16_t AttrCnt;

    if (MessageCode == RETCODE_OK)
    {
        AttrCnt            = StreamNum;
        PacketAttrHdSize1  = sizeof(SysPkgAttrHeader) + sizeof(SysPkgEncodeCfg);
    }
    else
    {
        AttrCnt           = 1;
        PacketAttrHdSize1 = sizeof(SysPkgAttrHeader) + sizeof(SysPkgMessageCode);
    }
    AppPacketSize    = PacketHeaderSize + PacketAttrHdSize1*AttrCnt;
    PayloadTotalSize = PacketAttrHdSize1*AttrCnt + sizeof(SdkPkgTransferProtocolKey);

    ReferrencePtr<SystemPacket> Reply( BaseMemoryManager::Instance().New<SystemPacket>() );
    if (NULL == Reply.GetPtr())
    {
        SYS_ERROR("Reply new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    Result = Reply->AllocatePacketBuffer(0, PayloadTotalSize);
    if (FAILED(Result))
    {
        SYS_ERROR("PakcetBuffer allocate fail\n");
        return Result;
    }

    //fill packet header
    Result = SystemPacket::FillPacketHeader(
                 Reply->GetPacketHeaderBuffer(),
                 CommandPacket->GetMessageTag(),
                 CommandPacket->GetVersion(),
                 SYSCODE_GET_ENCODECFG_RSP,
                 AttrCnt,
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
    uint16_t Offset        = 0;
    SysPkgAttrHeader *SysPkgAtrrHdPtr;

    if (MessageCode == 0)
    {
        for (int32_t Id = 0; Id < AttrCnt; Id++)
        {
            SysPkgAtrrHdPtr           = (SysPkgAttrHeader*)(PayloadBuffer + Offset);
            SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_ENCODECFG);
            SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
            Offset                   += sizeof(SysPkgAttrHeader);
            memcpy((PayloadBuffer + Offset), &((SysEncodeCfgPtr.GetPtr())[Id]), sizeof(SysPkgEncodeCfg));
            Offset                   += sizeof(SysPkgEncodeCfg);
        }
    }
    else
    {
        SysPkgMessageCode *SysPkgMessageCodePtr;

        SysPkgAtrrHdPtr           = (SysPkgAttrHeader*)PayloadBuffer;
        SysPkgAtrrHdPtr->s_Type   = HOST_TO_NETWORK_USHORT(TYPE_MESSAGECODE);
        SysPkgAtrrHdPtr->s_Length = HOST_TO_NETWORK_USHORT(PacketAttrHdSize1);
        Offset                   += sizeof( SysPkgAttrHeader );
        SysPkgMessageCodePtr                = (SysPkgMessageCode*)(PayloadBuffer + Offset);
        SysPkgMessageCodePtr->s_MessageCode = HOST_TO_NETWORK_UINT(MessageCode);
        SysPkgMessageCodePtr->s_MessageLen  = 0;
        Offset                   += sizeof(SysPkgMessageCode);
        Offset                   += SysPkgMessageCodePtr->s_MessageLen;
    }

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

    SYS_INFO("%s:%s normal out..........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT  SysGetEncodeConfigCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit(m_Session);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}


