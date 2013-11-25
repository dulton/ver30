#include "media_center_stop_codec_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterStopCodecCommandExecutor::MediaCenterStopCodecCommandExecutor(void)
    : MediaCenterOperationWithoutParameterCommandExecutor( GMI_STOP_CODEC, CT_ShortTask )
{
}

MediaCenterStopCodecCommandExecutor::~MediaCenterStopCodecCommandExecutor(void)
{
}

GMI_RESULT MediaCenterStopCodecCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterStopCodecCommandExecutor> StopCodecCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterStopCodecCommandExecutor>() );
    if ( NULL == StopCodecCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = StopCodecCommandExecutor;
    return MediaCenterOperationWithoutParameterCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterStopCodecCommandExecutor::Execute()
{
    FD_HANDLE CodecHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->StopCodec( CodecHandle );

    return MediaCenterOperationWithoutParameterCommandExecutor::Execute();
}
