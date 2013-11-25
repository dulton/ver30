#include "media_center_set_auto_focus_step_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterSetAutoFocusStepCommandExecutor::MediaCenterSetAutoFocusStepCommandExecutor()
    : MediaCenterSetOperationWithOneParameterCommandExecutor( GMI_SET_AUTO_FOCUS_STEP, CT_ShortTask )
{
}

MediaCenterSetAutoFocusStepCommandExecutor::~MediaCenterSetAutoFocusStepCommandExecutor(void)
{
}

GMI_RESULT MediaCenterSetAutoFocusStepCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterSetAutoFocusStepCommandExecutor> SetAutoFocusStepCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetAutoFocusStepCommandExecutor>() );
    if ( NULL == SetAutoFocusStepCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = SetAutoFocusStepCommandExecutor;
    return MediaCenterSetOperationWithOneParameterCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterSetAutoFocusStepCommandExecutor::Execute()
{
    FD_HANDLE AutoFocusHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->SetAutoFocusStep( AutoFocusHandle, (int32_t) m_Parameter1 );

    return MediaCenterSetOperationWithOneParameterCommandExecutor::Execute();
}
