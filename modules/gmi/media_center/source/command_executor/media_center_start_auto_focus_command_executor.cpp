#include "media_center_start_auto_focus_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterStartAutoFocusCommandExecutor::MediaCenterStartAutoFocusCommandExecutor()
    : MediaCenterOperationWithoutParameterCommandExecutor( GMI_START_AUTO_FOCUS, CT_ShortTask )
{
}

MediaCenterStartAutoFocusCommandExecutor::~MediaCenterStartAutoFocusCommandExecutor(void)
{
}

GMI_RESULT MediaCenterStartAutoFocusCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterStartAutoFocusCommandExecutor> StartAutoFocusCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterStartAutoFocusCommandExecutor>() );
    if ( NULL == StartAutoFocusCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = StartAutoFocusCommandExecutor;
    return MediaCenterOperationWithoutParameterCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterStartAutoFocusCommandExecutor::Execute()
{
    FD_HANDLE AutoFocusHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->StartAutoFocus( AutoFocusHandle );

    return MediaCenterOperationWithoutParameterCommandExecutor::Execute();
}
