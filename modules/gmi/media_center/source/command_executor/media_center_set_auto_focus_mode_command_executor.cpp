#include "media_center_set_auto_focus_mode_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterSetAutoFocusModeCommandExecutor::MediaCenterSetAutoFocusModeCommandExecutor()
    : MediaCenterSetOperationWithOneParameterCommandExecutor( GMI_SET_AUTO_FOCUS_MODE, CT_ShortTask )
{
}

MediaCenterSetAutoFocusModeCommandExecutor::~MediaCenterSetAutoFocusModeCommandExecutor(void)
{
}

GMI_RESULT MediaCenterSetAutoFocusModeCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterSetAutoFocusModeCommandExecutor> SetAutoFocusModeCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetAutoFocusModeCommandExecutor>() );
    if ( NULL == SetAutoFocusModeCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = SetAutoFocusModeCommandExecutor;
    return MediaCenterSetOperationWithOneParameterCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterSetAutoFocusModeCommandExecutor::Execute()
{
    FD_HANDLE AutoFocusHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->SetAutoFocusMode( AutoFocusHandle, (int32_t) m_Parameter1 );

    return MediaCenterSetOperationWithOneParameterCommandExecutor::Execute();
}
