#include "media_center_get_focus_position_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterGetFocusPositionCommandExecutor::MediaCenterGetFocusPositionCommandExecutor()
    : MediaCenterGetOperationWithOneParameterCommandExecutor( GMI_GET_FOCUS_POSITION, CT_ShortTask )
{
}

MediaCenterGetFocusPositionCommandExecutor::~MediaCenterGetFocusPositionCommandExecutor(void)
{
}

GMI_RESULT MediaCenterGetFocusPositionCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterGetFocusPositionCommandExecutor> GetFocusPositionCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterGetFocusPositionCommandExecutor>() );
    if ( NULL == GetFocusPositionCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = GetFocusPositionCommandExecutor;
    return MediaCenterGetOperationWithOneParameterCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterGetFocusPositionCommandExecutor::Execute()
{
    FD_HANDLE AutoFocusHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->GetFocusPosition( AutoFocusHandle, (int32_t *) &m_Parameter1 );

    return MediaCenterGetOperationWithOneParameterCommandExecutor::Execute();
}
