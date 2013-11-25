#include "media_center_get_zoom_position_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterGetZoomPositionCommandExecutor::MediaCenterGetZoomPositionCommandExecutor()
    : MediaCenterGetOperationWithOneParameterCommandExecutor( GMI_GET_ZOOM_POSITION, CT_ShortTask )
{
}

MediaCenterGetZoomPositionCommandExecutor::~MediaCenterGetZoomPositionCommandExecutor(void)
{
}

GMI_RESULT MediaCenterGetZoomPositionCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterGetZoomPositionCommandExecutor> GetZoomPositionCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterGetZoomPositionCommandExecutor>() );
    if ( NULL == GetZoomPositionCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = GetZoomPositionCommandExecutor;
    return MediaCenterGetOperationWithOneParameterCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterGetZoomPositionCommandExecutor::Execute()
{
    FD_HANDLE AutoFocusHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->GetZoomPosition( AutoFocusHandle, (int32_t *) &m_Parameter1 );

    return MediaCenterGetOperationWithOneParameterCommandExecutor::Execute();
}
