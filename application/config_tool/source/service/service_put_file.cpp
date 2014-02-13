#include "service_put_file.h"
#include "service_dispatch.h"
#include "configure_service.h"
#include "application.h"
#include "daemon_service.h"

#define TEMP_FILE_PATH   "/tmp/config_tool_put_file.temp"

// ACK XML string
static const uint8_t  l_XML_ACK[] = "<ACK operation=\"PutFile\"/>";

// Create service
static ServicePutFile l_ServicePutFileInstance;

template <>
ServicePutFile * Singleton<ServicePutFile>::ms_InstancePtr = NULL;

ServicePutFile::ServicePutFile()
    : m_TransHandle(GTP_INVALID_HANDLE)
    , m_FilePath()
    , m_MD5Value()
    , m_Compressed(false)
    , m_Encrypted(false)
    , m_NeedReserved(false)
    , m_NeedReboot(false)
    , m_UsingSupper(false)
    , m_State(eIdle)
    , m_File(NULL)
    , m_MD5Engine()
{
    ServiceDispatch::GetInstance().Register(this, this);
}

ServicePutFile::~ServicePutFile()
{
    ServiceDispatch::GetInstance().Unregister(this);

    // Recycle resources of thread, if thread is still running
    if (IsRunning())
    {
        Wait();
    }
}

void_t ServicePutFile::Execute(GtpTransHandle Handle, IParser * Parser)
{
    // Ignore parser, because parser is itself

    GMI_RESULT RetVal = GMI_SUCCESS;

    if (!StateIdle() || IsRunning())
    {
        PRINT_LOG(WARNING, "Service is busy now");

        // Send error response
        RetVal = SendSimpleResponse(Handle, "PutFile", GMI_SYSTEM_BUSY);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to send response");
            GtpDestroyTransHandle(Handle);
        }

        return;
    }

    do
    {
        if (!m_UsingSupper)
        {
            PRINT_LOG(WARNING, "Service must use supper user");
            RetVal = GMI_NO_ACCESS_RIGHT;
            break;
        }

        // Make sure temp file is removed
        unlink(TEMP_FILE_PATH);

        m_File = fopen(TEMP_FILE_PATH, "wb");
        if (NULL == m_File)
        {
            PRINT_LOG(ERROR, "Failed to create file %s, errno = %d", TEMP_FILE_PATH, errno);
            RetVal = GMI_OPEN_DEVICE_FAIL;
            break;
        }

        // Reset md5 engine
        m_MD5Engine.Reset();

        // Send ACK
        RetVal = GtpTransSendData(Handle, l_XML_ACK, sizeof(l_XML_ACK), GTP_DATA_TYPE_UTF8_XML);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to send ACK");
            break;
        }

        SwitchState(eSendingACK);

        // Bind transaction to this service
        Bind(Handle);
        return;
    } while (0); 

    if (m_File != NULL)
    {
        fclose(m_File);
        m_File = NULL;

        unlink(TEMP_FILE_PATH);
    }

    // Cancel task
    Application::GetSingleton().CancelDelayTask(ServicePutFile::OnTimeProc, this);

    // Send error response
    RetVal = SendSimpleResponse(Handle, "PutFile", RetVal);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to send response");
        GtpDestroyTransHandle(Handle);
    }
}

void_t ServicePutFile::OnSent(GtpTransHandle Handle, GMI_RESULT Errno)
{
    ASSERT_EQUAL(Handle, m_TransHandle);

    if (Errno != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to send data packet");
        GtpDestroyTransHandle(Handle);
        return;
    }

    switch (m_State)
    {
    case eSendingACK:
        {
            // Add timer in 3 seconds
            GMI_RESULT RetVal = Application::GetSingleton().AddDelayTask(ServicePutFile::OnTimeProc, this, 3, 0);
            if (GMI_SUCCESS == RetVal)
            {
                SwitchState(eReceivingFile);
                break;
            }

            // Remove reboot flag if error happens
            m_NeedReboot = false;

            // Send error response
            RetVal = SendSimpleResponse(Handle, "PutFile", RetVal);
            if (GMI_SUCCESS == RetVal)
            {
                SwitchState(eSendingResponse);
                break;
            }

            PRINT_LOG(ERROR, "Failed to send response");
            GtpDestroyTransHandle(Handle);
            break;
        }

    case eSendingResponse:
        {
            // Check if we need to reboot system
            if (m_NeedReboot)
            {
                PRINT_LOG(INFO, "System need reboot");

                // Stop process
                Application::GetSingleton().Stop();

                // Reboot system
                DaemonService::GetInstance().RebootSystem();
            }

            GtpDestroyTransHandle(Handle);
            break;
        }

    default:
        {
            PRINT_LOG(ERROR, "Unexpected state of service put file");
            GtpDestroyTransHandle(Handle);
            break;
        }
    }
}

void_t ServicePutFile::OnRecv(GtpTransHandle Handle, const uint8_t * Buffer, uint32_t BufferLength, uint8_t Type)
{
    ASSERT_EQUAL(Handle, m_TransHandle);

    if (!StateReceivingFile())
    {
        PRINT_LOG(WARNING, "Service is not in state 'eReceivingFile'");

        // Unexpected state, dispatch it again
        Unbind(Handle);
        ServiceDispatch::GetInstance().Dispatch(Handle, Buffer, BufferLength, Type);
        return;
    }

    GMI_RESULT RetVal = GMI_SUCCESS;

    switch (Type)
    {
    case GTP_DATA_TYPE_BINARY:
        {
            if (fwrite(Buffer, BufferLength, 1, m_File) != 1)
            {
                PRINT_LOG(ERROR, "Failed to write data into temp file");
                RetVal = GMI_SYSTEM_ERROR;
                break;
            }

            // Fill data into md5 engine
            m_MD5Engine.Update(Buffer, BufferLength);

            // Update timer
            RetVal = Application::GetSingleton().UpdateDelayTask(ServicePutFile::OnTimeProc, this, 3, 0);
            if (RetVal != GMI_SUCCESS)
            {
                PRINT_LOG(ERROR, "Failed to update delay task");
                break;
            }

            return;
        }

    case GTP_DATA_TYPE_UTF8_XML:
        {
            if (!ParseACK(Buffer, BufferLength))
            {
                // Not the ACK for put file, dispatch it again
                Unbind(Handle);
                ServiceDispatch::GetInstance().Dispatch(Handle, Buffer, BufferLength, Type);
                return;
            }

            // Close temp file
            fclose(m_File);
            m_File = NULL;

            // Finalize m5d engine
            m_MD5Engine.Finalize();

            // Get md5 value
            char_t MD5Value[33];
            m_MD5Engine.HexDigest(MD5Value, sizeof(MD5Value));

            // Check md5 value
            if (m_MD5Value != MD5Value)
            {
                PRINT_LOG(WARNING, "MD5 value not matched");
                DUMP_VARIABLE(m_MD5Value.c_str());
                DUMP_VARIABLE(MD5Value);
                RetVal = GMI_NO_AVAILABLE_DATA;
                break;
            }

            // Update timer in 100ms
            RetVal = Application::GetSingleton().UpdateDelayTask(ServicePutFile::OnTimeProc, this, 0, 100000);
            if (RetVal != GMI_SUCCESS)
            {
                PRINT_LOG(ERROR, "Failed to update delay task");
                break;
            }

            RetVal = Create();
            if (RetVal != GMI_SUCCESS)
            {
                PRINT_LOG(ERROR, "Failed to create thread");
                break;
            }

            SwitchState(eProcessingFile);
            return;
        }

    default:
        {
            PRINT_LOG(ERROR, "Unsupported data type");

            // Unsupported data type, dispatch it again
            Unbind(Handle);
            ServiceDispatch::GetInstance().Dispatch(Handle, Buffer, BufferLength, Type);
            return;
        }
    }

    // Cancel task
    Application::GetSingleton().CancelDelayTask(ServicePutFile::OnTimeProc, this);

    // Remove reboot flag if error happens
    m_NeedReboot = false;

    // Send error response
    RetVal = SendSimpleResponse(Handle, "PutFile", RetVal);
    if (GMI_SUCCESS == RetVal)
    {
        SwitchState(eSendingResponse);
        return;
    }

    PRINT_LOG(ERROR, "Failed to send response");
    GtpDestroyTransHandle(Handle);
}

const char_t * ServicePutFile::Name() const
{
    return "PutFile";
}

boolean_t ServicePutFile::ParseBinary(const uint8_t * Buffer, uint32_t BufferLength)
{
    // We could not parse binary data
    return false;
}

boolean_t ServicePutFile::ParseXML(TiXmlDocument & XmlDoc)
{
    // Check if root element is existed
    TiXmlElement * XmlRootElement = XmlDoc.RootElement();
    if (NULL == XmlRootElement)
    {
        PRINT_LOG(WARNING, "No root element");
        return false;
    }

    const char_t * Tag = XmlRootElement->Value();
    if (NULL == Tag || strcmp(Tag, "Request") != 0)
    {
        PRINT_LOG(VERBOSE, "Not a request");
        return false;
    }

    const char_t * Operation = XmlRootElement->Attribute("operation");
    if (NULL == Operation || strcmp(Operation, "PutFile") != 0)
    {
        PRINT_LOG(VERBOSE, "Not a put file request");
        return false;
    }

    PRINT_LOG(VERBOSE, "It is a put file request");

    if (!StateIdle() || IsRunning())
    {
        PRINT_LOG(VERBOSE, "Service is busy now, drop the data");
        return true;
    }

    PRINT_LOG(VERBOSE, "Parsing the file information in XML ...");

    const char * ToolIdString = XmlRootElement->Attribute("toolid");
    if (ToolIdString != NULL && strcmp(ToolIdString, "product") == 0)
    {
        m_UsingSupper = true;
    }
    else
    {
        m_UsingSupper = false;
    }

    TiXmlElement * XmlFileInfo = GetXmlElement(XmlRootElement, "FileInfo");
    if (NULL == XmlFileInfo)
    {
        PRINT_LOG(WARNING, "Failed to get 'FileInfo' element");
        return false;
    }

    int32_t Value = 0;
    if (XmlFileInfo->Attribute("compress", &Value) != NULL)
    {
        m_Compressed = (Value == 1);
    }
    else
    {
        m_Compressed = false;
    }

    if (XmlFileInfo->Attribute("encrypted", &Value) != NULL)
    {
        m_Encrypted = (Value == 1);
    }
    else
    {
        m_Encrypted = false;
    }

    if (XmlFileInfo->Attribute("reserve", &Value) != NULL)
    {
        m_NeedReserved = (Value == 1);
    }
    else
    {
        m_NeedReserved = false;
    }

    if (XmlFileInfo->Attribute("reboot", &Value) != NULL)
    {
        m_NeedReboot = (Value == 1);
    }
    else
    {
        m_NeedReboot = false;
    }

    TiXmlElement * XmlElement = GetXmlElement(XmlFileInfo, "Path");
    if (NULL == XmlElement || NULL == XmlElement->GetText())
    {
        PRINT_LOG(WARNING, "Failed to get path");
        return false;
    }

    m_FilePath = XmlElement->GetText();
    if (m_FilePath[m_FilePath.length() - 1] != '/')
    {
        m_FilePath += "/";
    }

    XmlElement = GetXmlElement(XmlFileInfo, "Filename");
    if (NULL == XmlElement || NULL == XmlElement->GetText())
    {
        PRINT_LOG(WARNING, "Failed to get filename");
        return false;
    }

    m_FilePath += XmlElement->GetText();

    XmlElement = GetXmlElement(XmlFileInfo, "Md5");
    if (NULL == XmlElement || NULL == XmlElement->GetText())
    {
        PRINT_LOG(WARNING, "Failed to get md5 value");
        return false;
    }

    m_MD5Value = XmlElement->GetText();

    return true;
}

void_t ServicePutFile::Bind(GtpTransHandle Handle)
{
    GtpTransSetReferenceInstance(Handle, this);

    if (GTP_INVALID_HANDLE != m_TransHandle)
    {
        PRINT_LOG(ERROR, "Othre transaction is already attached to service put file");
        DUMP_VARIABLE(Handle);
        return;
    }

    m_TransHandle = Handle;
}

void_t ServicePutFile::Unbind(GtpTransHandle Handle)
{
    GtpTransSetReferenceInstance(Handle, NULL);

    if (m_TransHandle != Handle)
    {
        PRINT_LOG(ERROR, "This transaction is not attached to service put file");
        DUMP_VARIABLE(Handle);
        return;
    }

    m_TransHandle = GTP_INVALID_HANDLE;

    if (StateReceivingFile())
    {
        // Cancel task
        Application::GetSingleton().CancelDelayTask(ServicePutFile::OnTimeProc, this);
    }

    // Switch service state to idle
    SwitchState(eIdle);

    // Close temp file if it is opened
    if (m_File != NULL)
    {
        fclose(m_File);
        m_File = NULL;

        // Remove temp file
        unlink(TEMP_FILE_PATH);
    }

    // Reset md5 engine
    m_MD5Engine.Reset();
}

void_t ServicePutFile::OnTimeProc(void_t * Data)
{
    ASSERT_VALID_POINT(Data);
    ServicePutFile * This = static_cast<ServicePutFile *>(Data);
    This->OnTime();
}

void_t ServicePutFile::OnTime()
{
    GMI_RESULT RetVal = GMI_SUCCESS;

    switch (m_State)
    {
    // If command canceled when processing file, then state will change to idle
    case eIdle:
    case eProcessingFile:
        {
            // No thing to do
            break;
        }

    case eReceivingFile:
        {
            PRINT_LOG(WARNING, "Receive file timed out");

            // Remove reboot flag if error happens
            m_NeedReboot = false;

            // Send error response
            RetVal = SendSimpleResponse(m_TransHandle, "PutFile", GMI_WAIT_TIMEOUT);
            if (GMI_SUCCESS == RetVal)
            {
                SwitchState(eSendingResponse);
                return;
            }

            PRINT_LOG(ERROR, "Failed to send response");
            GtpDestroyTransHandle(m_TransHandle);
            return;
        }

    default:
        {
            PRINT_LOG(ERROR, "Unexpected state of service put file");
            GtpDestroyTransHandle(m_TransHandle);
            return;
        }
    }

    ASSERT(IsRunning(), "Thread MUST be running");

    if (IsStopped())
    {
        GMI_RESULT ThreadRetVal = GMI_SUCCESS;

        // Recycle resouces of thread
        RetVal = Wait(&ThreadRetVal);
        if (GMI_SUCCESS == RetVal)
        {
            if (ThreadRetVal != GMI_SUCCESS)
            {
                PRINT_LOG(ERROR, "Failed to run thread");
                DUMP_VARIABLE(ThreadRetVal);

                // Remove reboot flag if error happens
                m_NeedReboot = false;
            }

            if (GTP_INVALID_HANDLE == m_TransHandle)
            {
                PRINT_LOG(WARNING, "No transaction is binding, no need to send response");
                return;
            }

            // Send error response
            RetVal = SendSimpleResponse(m_TransHandle, "PutFile", ThreadRetVal);
            if (GMI_SUCCESS == RetVal)
            {
                SwitchState(eSendingResponse);
                return;
            }

            PRINT_LOG(ERROR, "Failed to send response");
            GtpDestroyTransHandle(m_TransHandle);
        }
        else
        {
            PRINT_LOG(ALERT, "Faild to stop thread");
        }
    }
    else
    {
        // Try again in 100ms
        RetVal = Application::GetSingleton().AddDelayTask(ServicePutFile::OnTimeProc, this, 0, 100000);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to add delay task");

            // TODO:
        }
    }
}

GMI_RESULT ServicePutFile::ThreadEntry()
{
    GMI_RESULT RetVal = ConfigureService::GetInstance().ImportFile(TEMP_FILE_PATH, m_FilePath.c_str(), m_NeedReserved, 0);

    // Remove temp file
    unlink(TEMP_FILE_PATH);

    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(WARNING, "Failed to import file");
        return RetVal;
    }

    return GMI_SUCCESS;
}

boolean_t ServicePutFile::ParseACK(const uint8_t * Buffer, uint32_t BufferLength)
{
    std::string XmlString(reinterpret_cast<const char_t *>(Buffer), BufferLength);

    // Parse xml string
    TiXmlDocument XmlDoc;
    XmlDoc.Parse(XmlString.c_str());

    // Check if root element is existed
    TiXmlElement * XmlRootElement = XmlDoc.RootElement();
    if (NULL == XmlRootElement)
    {
        PRINT_LOG(WARNING, "No root element");
        return false;
    }

    const char_t * Tag = XmlRootElement->Value();
    if (NULL == Tag || strcmp(Tag, "ACK") != 0)
    {
        PRINT_LOG(VERBOSE, "Not a ACK");
        return false;
    }

    const char_t * Operation = XmlRootElement->Attribute("operation");
    if (NULL == Operation || strcmp(Operation, "PutFile") != 0)
    {
        PRINT_LOG(VERBOSE, "Not a put file ACK");
        return false;
    }

    PRINT_LOG(VERBOSE, "It is a put file ACK");
    return true;
}

