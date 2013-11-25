#include "media_center_control_auto_focus_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterControlAutoFocusCommandExecutor::MediaCenterControlAutoFocusCommandExecutor()
    : MediaCenterSetOperationWithOneParameterCommandExecutor( GMI_CONTROL_AUTO_FOCUS, CT_ShortTask )
{
}

MediaCenterControlAutoFocusCommandExecutor::~MediaCenterControlAutoFocusCommandExecutor(void)
{
}

GMI_RESULT MediaCenterControlAutoFocusCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterControlAutoFocusCommandExecutor> ControlAutoFocusCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterControlAutoFocusCommandExecutor>() );
    if ( NULL == ControlAutoFocusCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = ControlAutoFocusCommandExecutor;
    return MediaCenterSetOperationWithOneParameterCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterControlAutoFocusCommandExecutor::Execute()
{
    FD_HANDLE AutoFocusHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->ControlAutoFocus( AutoFocusHandle, (int8_t) m_Parameter1 );

    return MediaCenterSetOperationWithOneParameterCommandExecutor::Execute();
}
