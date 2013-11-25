#pragma once

#include "base_command_pipeline_manager.h"
#include "gmi_system_headers.h"

class BaseSessionManager
{
protected:
    BaseSessionManager(void);
public:
    virtual ~BaseSessionManager(void);

    virtual GMI_RESULT  Initialize( void_t *Argument, size_t ArgumentSize ) = 0;
    virtual GMI_RESULT  Deinitialize() = 0;
    virtual GMI_RESULT  Start( ReferrencePtr<BaseCommandPipelineManager> CommandPipeline ) = 0;
    virtual GMI_RESULT  Stop() = 0;

private:
    ReferrencePtr<BaseCommandPipelineManager> m_CommandPipeline;
};
