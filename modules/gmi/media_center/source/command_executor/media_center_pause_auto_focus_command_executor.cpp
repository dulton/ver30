#include "media_center_pause_auto_focus_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterPauseAutoFocusCommandExecutor::MediaCenterPauseAutoFocusCommandExecutor()
    : MediaCenterSetOperationWithOneParameterCommandExecutor( GMI_PAUSE_AUTO_FOCUS, CT_ShortTask )
{
}

MediaCenterPauseAutoFocusCommandExecutor::~MediaCenterPauseAutoFocusCommandExecutor(void)
{
}

GMI_RESULT MediaCenterPauseAutoFocusCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterPauseAutoFocusCommandExecutor> PauseAutoFocusCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterPauseAutoFocusCommandExecutor>() );
    if ( NULL == PauseAutoFocusCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = PauseAutoFocusCommandExecutor;
    return MediaCenterSetOperationWithOneParameterCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterPauseAutoFocusCommandExecutor::Execute()
{
    FD_HANDLE AutoFocusHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->PauseAutoFocus( AutoFocusHandle, (int8_t) m_Parameter1 );

    return MediaCenterSetOperationWithOneParameterCommandExecutor::Execute();
}
