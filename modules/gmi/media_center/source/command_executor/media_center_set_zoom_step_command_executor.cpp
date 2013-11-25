#include "media_center_set_zoom_step_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterSetZoomStepCommandExecutor::MediaCenterSetZoomStepCommandExecutor()
    : MediaCenterSetOperationWithOneParameterCommandExecutor( GMI_SET_ZOOM_STEP, CT_ShortTask )
{
}

MediaCenterSetZoomStepCommandExecutor::~MediaCenterSetZoomStepCommandExecutor(void)
{
}

GMI_RESULT MediaCenterSetZoomStepCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterSetZoomStepCommandExecutor> SetZoomStepCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetZoomStepCommandExecutor>() );
    if ( NULL == SetZoomStepCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = SetZoomStepCommandExecutor;
    return MediaCenterSetOperationWithOneParameterCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterSetZoomStepCommandExecutor::Execute()
{
    FD_HANDLE AutoFocusHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->SetZoomStep( AutoFocusHandle, (int32_t) m_Parameter1 );

    return MediaCenterSetOperationWithOneParameterCommandExecutor::Execute();
}
