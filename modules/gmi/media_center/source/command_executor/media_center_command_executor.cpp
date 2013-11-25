#include "media_center_command_executor.h"

MediaCenterCommandExecutor::MediaCenterCommandExecutor( uint32_t CommandId, enum CommandType Type )
    : BaseCommandExecutor( CommandId, Type )
    , m_MediaCenter()
    , m_Result( GMI_FAIL )
{
}

MediaCenterCommandExecutor::~MediaCenterCommandExecutor(void)
{
}

GMI_RESULT MediaCenterCommandExecutor::Reply()
{
    GMI_RESULT Result =  m_Reply->Submit( m_Session );
    if ( FAILED( Result ) )
    {
        return Result;
    }

    return GMI_SUCCESS;
}

void_t MediaCenterCommandExecutor::SetParameter( ReferrencePtr<MediaCenter>& Center )
{
    m_MediaCenter = Center;
}
