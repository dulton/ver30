#include "media_center_control_zoom_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterControlZoomCommandExecutor::MediaCenterControlZoomCommandExecutor()
    : MediaCenterSetOperationWithOneParameterCommandExecutor( GMI_CONTROL_ZOOM, CT_ShortTask )
{
}

MediaCenterControlZoomCommandExecutor::~MediaCenterControlZoomCommandExecutor(void)
{
}

GMI_RESULT MediaCenterControlZoomCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterControlZoomCommandExecutor> ControlZoomCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterControlZoomCommandExecutor>() );
    if ( NULL == ControlZoomCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = ControlZoomCommandExecutor;
    return MediaCenterSetOperationWithOneParameterCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterControlZoomCommandExecutor::Execute()
{
    FD_HANDLE AutoFocusHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->ControlZoom( AutoFocusHandle, (int8_t) m_Parameter1 );

    return MediaCenterSetOperationWithOneParameterCommandExecutor::Execute();
}
