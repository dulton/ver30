#include "media_center_stop_auto_focus_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterStopAutoFocusCommandExecutor::MediaCenterStopAutoFocusCommandExecutor()
    : MediaCenterOperationWithoutParameterCommandExecutor( GMI_STOP_AUTO_FOCUS, CT_ShortTask )
{
}

MediaCenterStopAutoFocusCommandExecutor::~MediaCenterStopAutoFocusCommandExecutor(void)
{
}

GMI_RESULT MediaCenterStopAutoFocusCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterStopAutoFocusCommandExecutor> StopAutoFocusCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterStopAutoFocusCommandExecutor>() );
    if ( NULL == StopAutoFocusCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = StopAutoFocusCommandExecutor;
    return MediaCenterOperationWithoutParameterCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterStopAutoFocusCommandExecutor::Execute()
{
    FD_HANDLE AutoFocusHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->StopAutoFocus( AutoFocusHandle );

    return MediaCenterOperationWithoutParameterCommandExecutor::Execute();
}
