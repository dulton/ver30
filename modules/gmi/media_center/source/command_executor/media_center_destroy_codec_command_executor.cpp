#include "media_center_destroy_codec_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterDestroyCodecCommandExecutor::MediaCenterDestroyCodecCommandExecutor(void)
    : MediaCenterCloseDeviceCommandExecutor( GMI_DESTROY_CODEC, CT_ShortTask )
{
}

MediaCenterDestroyCodecCommandExecutor::~MediaCenterDestroyCodecCommandExecutor(void)
{
}

GMI_RESULT MediaCenterDestroyCodecCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterDestroyCodecCommandExecutor> DestroyCodecCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterDestroyCodecCommandExecutor>() );
    if ( NULL == DestroyCodecCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = DestroyCodecCommandExecutor;
    return MediaCenterCloseDeviceCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterDestroyCodecCommandExecutor::Execute()
{
    FD_HANDLE CodecHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->DestroyCodec( CodecHandle );

    return MediaCenterCloseDeviceCommandExecutor::Execute();
}
