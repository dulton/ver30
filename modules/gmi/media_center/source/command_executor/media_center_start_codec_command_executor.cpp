#include "media_center_start_codec_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterStartCodecCommandExecutor::MediaCenterStartCodecCommandExecutor(void)
    : MediaCenterOperationWithoutParameterCommandExecutor( GMI_START_CODEC, CT_ShortTask )
{
}

MediaCenterStartCodecCommandExecutor::~MediaCenterStartCodecCommandExecutor(void)
{
}

GMI_RESULT MediaCenterStartCodecCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterStartCodecCommandExecutor> StartCodecCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterStartCodecCommandExecutor>() );
    if ( NULL == StartCodecCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = StartCodecCommandExecutor;
    return MediaCenterOperationWithoutParameterCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterStartCodecCommandExecutor::Execute()
{
    FD_HANDLE CodecHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->StartCodec( CodecHandle );

    return MediaCenterOperationWithoutParameterCommandExecutor::Execute();
}
