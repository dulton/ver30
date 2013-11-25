#include "media_center_auto_focus_global_scan_command_executor.h"

#include "application_packet.h"
#include "media_center_packet.h"

MediaCenterAutoFocusGlobalScanCommandExecutor::MediaCenterAutoFocusGlobalScanCommandExecutor()
    : MediaCenterOperationWithoutParameterCommandExecutor( GMI_AUTO_FOCUS_GLOBAL_SCAN, CT_ShortTask )
{
}

MediaCenterAutoFocusGlobalScanCommandExecutor::~MediaCenterAutoFocusGlobalScanCommandExecutor(void)
{
}

GMI_RESULT MediaCenterAutoFocusGlobalScanCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    ApplicationPacket *MediaCenterPacket = (ApplicationPacket*) Packet.GetPtr();

    if ( (0 == (GMI_MESSAGE_TYPE_REQUEST & MediaCenterPacket->GetMessageType())) || ( GetCommandId() != MediaCenterPacket->GetMessageId() ) )
    {
        return GMI_FAIL;
    }

    SafePtr<MediaCenterAutoFocusGlobalScanCommandExecutor> AutoFocusGlobalScanCommandExecutor( BaseMemoryManager::Instance().New<MediaCenterAutoFocusGlobalScanCommandExecutor>() );
    if ( NULL == AutoFocusGlobalScanCommandExecutor.GetPtr() )
    {
        return GMI_OUT_OF_MEMORY;
    }

    CommandExecutor = AutoFocusGlobalScanCommandExecutor;
    return MediaCenterOperationWithoutParameterCommandExecutor::Create( Packet, CommandExecutor );
}

GMI_RESULT MediaCenterAutoFocusGlobalScanCommandExecutor::Execute()
{
    FD_HANDLE AutoFocusHandle = reinterpret_cast<FD_HANDLE> ((size_t)m_Token);
    m_Result = m_MediaCenter->AutoFocusGlobalScan( AutoFocusHandle );

    return MediaCenterOperationWithoutParameterCommandExecutor::Execute();
}
