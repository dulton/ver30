#include "media_center_reset_zoom_motor_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterResetZoomMotorCommandExecutor::MediaCenterResetZoomMotorCommandExecutor()
    : MediaCenterOperationWithoutParameterCommandExecutor( GMI_RESET_ZOOM_MOTOR, CT_ShortTask )
{
}

MediaCenterResetZoomMotorCommandExecutor::~MediaCenterResetZoomMotorCommandExecutor(void)
{
}

GMI_RESULT MediaCenterResetZoomMotorCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterResetZoomMotorCommandExecutor> ResetZoomMotorCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterResetZoomMotorCommandExecutor>() );
    if ( NULL == ResetZoomMotorCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = ResetZoomMotorCommandExecutor;
    return MediaCenterOperationWithoutParameterCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterResetZoomMotorCommandExecutor::Execute()
{
    FD_HANDLE ZoomHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->ResetZoomMotor( ZoomHandle );

    return MediaCenterOperationWithoutParameterCommandExecutor::Execute();
}
