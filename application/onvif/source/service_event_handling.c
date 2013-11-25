#include "log.h"
#include "threads.h"
#include "service_utilitly.h"
#include "soapH.h"
#include "sys_env_types.h"
#include "gmi_system_headers.h"

SOAP_FMAC5 int SOAP_FMAC6 __ns3__GetEventProperties(struct soap* soap_ptr, struct _tev__GetEventProperties *tev__GetEventProperties, struct _tev__GetEventPropertiesResponse *tev__GetEventPropertiesResponse)
{

    soap_ptr->header->wsa5__Action = soap_strdup(soap_ptr, "http://www.onvif.org/ver10/events/wsdl/EventPortType/GetEventPropertiesResponse");

    tev__GetEventPropertiesResponse->__sizeTopicNamespaceLocation = 1;
    tev__GetEventPropertiesResponse->TopicNamespaceLocation = (char **)soap_malloc_zero( soap_ptr, sizeof(char*) * tev__GetEventPropertiesResponse->__sizeTopicNamespaceLocation);
    *(tev__GetEventPropertiesResponse->TopicNamespaceLocation) = soap_strdup(soap_ptr, "http://www.onvif.org/onvif/ver10/topics/topicns.xml");
    tev__GetEventPropertiesResponse->wsnt__FixedTopicSet = xsd__boolean__true_;
    tev__GetEventPropertiesResponse->wstop__TopicSet = (struct wstop__TopicSetType *)soap_malloc_zero( soap_ptr, sizeof(struct wstop__TopicSetType));

    tev__GetEventPropertiesResponse->wstop__TopicSet->__size = 1;
    tev__GetEventPropertiesResponse->wstop__TopicSet->__any = (char **)soap_malloc_zero( soap_ptr, sizeof(char*) * tev__GetEventPropertiesResponse->wstop__TopicSet->__size);
    *(tev__GetEventPropertiesResponse->wstop__TopicSet->__any) = soap_strdup(soap_ptr, "<tns1:UserAlarm wstop:topic=\"true\"><tt:MessageDescription><tt:Source><tt:SimpleItemDescription Name=\"VideoSourceConfigurationToken\" Type=\"tt:ReferenceToken\"/></tt:Source><tt:Data><tt:SimpleItemDescription Name=\"AlarmType\" Type=\"xs:string\"/></tt:Data></tt:MessageDescription></tns1:UserAlarm>");

    tev__GetEventPropertiesResponse->__sizeTopicExpressionDialect = 2;
    tev__GetEventPropertiesResponse->wsnt__TopicExpressionDialect = (char **)soap_malloc_zero( soap_ptr, sizeof(char*) * tev__GetEventPropertiesResponse->__sizeTopicExpressionDialect);
    tev__GetEventPropertiesResponse->wsnt__TopicExpressionDialect[1] = soap_strdup(soap_ptr, "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet");
    tev__GetEventPropertiesResponse->wsnt__TopicExpressionDialect[0] = soap_strdup(soap_ptr, "http://docs.oasis-open.org/wsn/t-1/TopicExpression/Concrete");


    tev__GetEventPropertiesResponse->__sizeMessageContentFilterDialect = 1;
    tev__GetEventPropertiesResponse->MessageContentFilterDialect = (char **)soap_malloc_zero( soap_ptr, sizeof(char*) * tev__GetEventPropertiesResponse->__sizeMessageContentFilterDialect);
    *(tev__GetEventPropertiesResponse->MessageContentFilterDialect) = soap_strdup(soap_ptr, "http://www.onvif.org/ver10/tev/messageContentFilter/ItemFilter");


    tev__GetEventPropertiesResponse->__sizeMessageContentSchemaLocation = 1;
    tev__GetEventPropertiesResponse->MessageContentSchemaLocation = (char **)soap_malloc_zero( soap_ptr, sizeof(char*) * tev__GetEventPropertiesResponse->__sizeMessageContentSchemaLocation);
    *(tev__GetEventPropertiesResponse->MessageContentSchemaLocation) = soap_strdup(soap_ptr, "http://www.onvif.org/onvif/ver10/schema/onvif.xsd");


    return SOAP_OK;
}
