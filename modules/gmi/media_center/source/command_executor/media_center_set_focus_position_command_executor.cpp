#include "media_center_set_focus_position_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterSetFocusPositionCommandExecutor::MediaCenterSetFocusPositionCommandExecutor()
    : MediaCenterSetOperationWithOneParameterCommandExecutor( GMI_SET_FOCUS_POSITION, CT_ShortTask )
{
}

MediaCenterSetFocusPositionCommandExecutor::~MediaCenterSetFocusPositionCommandExecutor(void)
{
}

GMI_RESULT MediaCenterSetFocusPositionCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterSetFocusPositionCommandExecutor> SetFocusPositionCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterSetFocusPositionCommandExecutor>() );
    if ( NULL == SetFocusPositionCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = SetFocusPositionCommandExecutor;
    return MediaCenterSetOperationWithOneParameterCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterSetFocusPositionCommandExecutor::Execute()
{
    FD_HANDLE AutoFocusHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->SetFocusPosition( AutoFocusHandle, (int32_t) m_Parameter1 );

    return MediaCenterSetOperationWithOneParameterCommandExecutor::Execute();
}
