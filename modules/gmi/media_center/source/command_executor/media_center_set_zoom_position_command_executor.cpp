#include "media_center_set_zoom_position_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterSetZoomPositionCommandExecutor::MediaCenterSetZoomPositionCommandExecutor()
    : MediaCenterSetOperationWithOneParameterCommandExecutor( GMI_SET_ZOOM_POSITION, CT_ShortTask )
{
}

MediaCenterSetZoomPositionCommandExecutor::~MediaCenterSetZoomPositionCommandExecutor(void)
{
}

GMI_RESULT MediaCenterSetZoomPositionCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterSetZoomPositionCommandExecutor> SetZoomPositionCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetZoomPositionCommandExecutor>() );
    if ( NULL == SetZoomPositionCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = SetZoomPositionCommandExecutor;
    return MediaCenterSetOperationWithOneParameterCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterSetZoomPositionCommandExecutor::Execute()
{
    FD_HANDLE AutoFocusHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->SetZoomPosition( AutoFocusHandle, (int32_t) m_Parameter1 );

    return MediaCenterSetOperationWithOneParameterCommandExecutor::Execute();
}
