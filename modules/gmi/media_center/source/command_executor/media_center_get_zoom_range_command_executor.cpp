#include "media_center_get_zoom_range_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterGetZoomRangeCommandExecutor::MediaCenterGetZoomRangeCommandExecutor()
    : MediaCenterGetOperationWithTwoParameterCommandExecutor( GMI_GET_ZOOM_RANGE, CT_ShortTask )
{
}

MediaCenterGetZoomRangeCommandExecutor::~MediaCenterGetZoomRangeCommandExecutor(void)
{
}

GMI_RESULT MediaCenterGetZoomRangeCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterGetZoomRangeCommandExecutor> GetZoomRangeCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterGetZoomRangeCommandExecutor>() );
    if ( NULL == GetZoomRangeCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = GetZoomRangeCommandExecutor;
    return MediaCenterGetOperationWithTwoParameterCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterGetZoomRangeCommandExecutor::Execute()
{
    FD_HANDLE AutoFocusHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->GetFocusRange( AutoFocusHandle, (int32_t*) &m_Parameter1, (int32_t*) &m_Parameter2 );

    return MediaCenterGetOperationWithTwoParameterCommandExecutor::Execute();
}
