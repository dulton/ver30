#include "media_center_reset_focus_motor_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterResetFocusMotorCommandExecutor::MediaCenterResetFocusMotorCommandExecutor()
    : MediaCenterOperationWithoutParameterCommandExecutor( GMI_RESET_FOCUS_MOTOR, CT_ShortTask )
{
}

MediaCenterResetFocusMotorCommandExecutor::~MediaCenterResetFocusMotorCommandExecutor(void)
{
}

GMI_RESULT MediaCenterResetFocusMotorCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterResetFocusMotorCommandExecutor> ResetFocusMotorCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterResetFocusMotorCommandExecutor>() );
    if ( NULL == ResetFocusMotorCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = ResetFocusMotorCommandExecutor;
    return MediaCenterOperationWithoutParameterCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterResetFocusMotorCommandExecutor::Execute()
{
    FD_HANDLE AutoFocusHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->ResetFocusMotor( AutoFocusHandle );

    return MediaCenterOperationWithoutParameterCommandExecutor::Execute();
}
