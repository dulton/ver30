#pragma once

#include "gmi_system_headers.h"
#include "event_center.h"
#include "event_process_header.h"
#include "event_transaction_header.h"

class EventTransactionCenter
{
public:
    EventTransactionCenter();
    ~EventTransactionCenter(void);

    GMI_RESULT Start( const void_t *Parameter, size_t ParameterLength, EventCallback Callback, void_t *UserData );
    GMI_RESULT Stop();

    GMI_RESULT ConfigureGPIOAlarmInput( const void_t *Parameter, size_t ParameterLength );
    GMI_RESULT ConfigureGPIOAlarmOutput( const void_t *Parameter, size_t ParameterLength );

private:
    GMI_RESULT StartGPIOAlarmInput();
    GMI_RESULT StopGPIOAlarmInput();
    GMI_RESULT StartGPIOAlarmOutput();
    GMI_RESULT StopGPIOAlarmOutput();

private:
    ReferrencePtr<EventCenter>  m_Center;
    EventCallback               m_Callback;
    void_t                      *m_UserData;
};
