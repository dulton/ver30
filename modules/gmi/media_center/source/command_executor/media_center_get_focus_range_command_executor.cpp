#include "media_center_get_focus_range_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterGetFocusRangeCommandExecutor::MediaCenterGetFocusRangeCommandExecutor()
    : MediaCenterGetOperationWithTwoParameterCommandExecutor( GMI_GET_FOCUS_RANGE, CT_ShortTask )
{
}

MediaCenterGetFocusRangeCommandExecutor::~MediaCenterGetFocusRangeCommandExecutor(void)
{
}

GMI_RESULT MediaCenterGetFocusRangeCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterGetFocusRangeCommandExecutor> GetFocusRangeCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterGetFocusRangeCommandExecutor>() );
    if ( NULL == GetFocusRangeCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = GetFocusRangeCommandExecutor;
    return MediaCenterGetOperationWithTwoParameterCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterGetFocusRangeCommandExecutor::Execute()
{
    FD_HANDLE AutoFocusHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->GetFocusRange( AutoFocusHandle, (int32_t*) &m_Parameter1, (int32_t*) &m_Parameter2 );

    return MediaCenterGetOperationWithTwoParameterCommandExecutor::Execute();
}
