#include "service_get_file.h"
#include "service_dispatch.h"
#include "configure_service.h"
#include "application.h"

#define TEMP_FILE_PATH   "/tmp/config_tool_get_file.temp"

// ACK XML string
static const uint8_t  l_XML_ACK[] = "<ACK operation=\"GetFile\"/>";

// Create service
static ServiceGetFile l_ServiceGetFileInstance;

template <>
ServiceGetFile * Singleton<ServiceGetFile>::ms_InstancePtr = NULL;

ServiceGetFile::ServiceGetFile()
    : m_TransHandle(GTP_INVALID_HANDLE)
    , m_FileList()
    , m_UsingSupper(false)
    , m_State(eIdle)
    , m_File(NULL)
    , m_MD5Engine()
    , m_Compressed(false)
    , m_Encrypted(false)
{
    ServiceDispatch::GetInstance().Register(this, this);
}

ServiceGetFile::~ServiceGetFile()
{
    ServiceDispatch::GetInstance().Unregister(this);

    // Recycle resources of thread, if thread is still running
    if (IsRunning())
    {
        Wait();
    }
}

void_t ServiceGetFile::Execute(GtpTransHandle Handle, IParser * Parser)
{
    // Ignore parser, because parser is itself

    GMI_RESULT RetVal = GMI_SUCCESS;

    if (!StateIdle() || IsRunning())
    {
        PRINT_LOG(WARNING, "Service is busy now");

        // Send error response
        RetVal = SendSimpleResponse(Handle, "GetFile", GMI_SYSTEM_BUSY);
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

        // Add timer in 100ms
        RetVal = Application::GetSingleton().AddDelayTask(ServiceGetFile::OnTimeProc, this, 0, 100000);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to add delay task");
            break;
        }

        RetVal = Create();
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to create thread");
            break;
        }

        SwitchState(ePackingFile);

        // Bind transaction to this service
        Bind(Handle);
        return;
    } while (0);

    // Cancel task
    Application::GetSingleton().CancelDelayTask(ServiceGetFile::OnTimeProc, this);

    // Send error response
    RetVal = SendSimpleResponse(Handle, "GetFile", RetVal);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to send response");
        GtpDestroyTransHandle(Handle);
    }
}

void_t ServiceGetFile::OnSent(GtpTransHandle Handle, GMI_RESULT Errno)
{
    ASSERT_EQUAL(Handle, m_TransHandle);

    if (Errno != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to send data packet");
        GtpDestroyTransHandle(Handle);
        return;
    }

    GMI_RESULT RetVal = GMI_SUCCESS;

    switch (m_State)
    {
    case eSendingACK:
        {
            // Add timer in 3 seconds
            RetVal = Application::GetSingleton().AddDelayTask(ServiceGetFile::OnTimeProc, this, 3, 0);
            if (GMI_SUCCESS == RetVal)
            {
                SwitchState(eReceivingACK);
                break;
            }

            // Send error response
            RetVal = SendSimpleResponse(Handle, "GetFile", RetVal);
            if (GMI_SUCCESS == RetVal)
            {
                SwitchState(eSendingResponse);
                break;
            }

            PRINT_LOG(ERROR, "Failed to send response");
            GtpDestroyTransHandle(Handle);
            break;
        }

    case eSendingFile:
        {
            // Send next packet
            RetVal = SendNextPacket();
            if (GMI_SUCCESS == RetVal)
            {
                break;
            }

            PRINT_LOG(ERROR, "Failed to send packet");
            GtpDestroyTransHandle(Handle);
            break;
        }

    case eSendingResponse:
        {
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

void_t ServiceGetFile::OnRecv(GtpTransHandle Handle, const uint8_t * Buffer, uint32_t BufferLength, uint8_t Type)
{
    ASSERT_EQUAL(Handle, m_TransHandle);

    if (!StateReceivingACK())
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
    case GTP_DATA_TYPE_UTF8_XML:
        {
            if (!ParseACK(Buffer, BufferLength))
            {
                // Not the ACK for get file, dispatch it again
                Unbind(Handle);
                ServiceDispatch::GetInstance().Dispatch(Handle, Buffer, BufferLength, Type);
                return;
            }

            // Cancel task
            Application::GetSingleton().CancelDelayTask(ServiceGetFile::OnTimeProc, this);

            // Open temp file
            m_File = fopen(TEMP_FILE_PATH, "rb");
            if (NULL == m_File)
            {
                PRINT_LOG(ERROR, "Failed to create file %s, errno = %d", TEMP_FILE_PATH, errno);
                RetVal = GMI_OPEN_DEVICE_FAIL;
                break;
            }

            // Reset md5 engine
            m_MD5Engine.Reset();

            // Start sending file
            RetVal = SendNextPacket();
            if (RetVal != GMI_SUCCESS)
            {
                PRINT_LOG(ERROR, "Failed to send packet");
                break;
            }

            SwitchState(eSendingFile);
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

    // Closetemp file, if it is opened
    if (m_File != NULL)
    {
        fclose(m_File);
        m_File = NULL;
    }

    // Remove temp file
    unlink(TEMP_FILE_PATH);

    // Send error response
    RetVal = SendSimpleResponse(Handle, "GetFile", RetVal);
    if (GMI_SUCCESS == RetVal)
    {
        SwitchState(eSendingResponse);
        return;
    }

    PRINT_LOG(ERROR, "Failed to send response");
    GtpDestroyTransHandle(Handle);
}

const char_t * ServiceGetFile::Name() const
{
    return "GetFile";
}

boolean_t ServiceGetFile::ParseBinary(const uint8_t * Buffer, uint32_t BufferLength)
{
    // We could not parse binary data
    return false;
}

boolean_t ServiceGetFile::ParseXML(TiXmlDocument & XmlDoc)
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
    if (NULL == Operation || strcmp(Operation, "GetFile") != 0)
    {
        PRINT_LOG(VERBOSE, "Not a get file request");
        return false;
    }

    PRINT_LOG(VERBOSE, "It is a get file request");

    if (!StateIdle() || IsRunning())
    {
        PRINT_LOG(VERBOSE, "Service is busy now, drop the data");
        return true;
    }

    PRINT_LOG(VERBOSE, "Parsing the file list in XML ...");

    const char * ToolIdString = XmlRootElement->Attribute("toolid");
    if (ToolIdString != NULL && strcmp(ToolIdString, "product") == 0)
    {
        m_UsingSupper = true;
    }
    else
    {
        m_UsingSupper = false;
    }

    TiXmlElement * XmlFileList = GetXmlElement(XmlRootElement, "FileList");
    if (NULL == XmlFileList)
    {
        PRINT_LOG(WARNING, "Failed to get 'FileInfo' element");
        return false;
    }

    // Clear current file list
    m_FileList.clear();

    for (TiXmlNode * XmlNode = XmlFileList->FirstChild("File"); XmlNode != NULL; XmlNode = XmlFileList->IterateChildren("File", XmlNode))
    {
        TiXmlElement * XmlFile = XmlNode->ToElement();
        if (XmlFile != NULL && XmlFile->GetText() != NULL)
        {
            if (access(XmlFile->GetText(), R_OK) != 0)
            {
                PRINT_LOG(WARNING, "File %s is not readable", XmlFile->GetText());
                continue;
            }

            PRINT_LOG(VERBOSE, "Add file %s to list", XmlFile->GetText());
            m_FileList.push_back(XmlFile->GetText());
        }
    }

    return true;
}

void_t ServiceGetFile::Bind(GtpTransHandle Handle)
{
    GtpTransSetReferenceInstance(Handle, this);

    if (GTP_INVALID_HANDLE != m_TransHandle)
    {
        PRINT_LOG(ERROR, "Othre transaction is already attached to service get file");
        DUMP_VARIABLE(Handle);
        return;
    }

    m_TransHandle = Handle;
}

void_t ServiceGetFile::Unbind(GtpTransHandle Handle)
{
    GtpTransSetReferenceInstance(Handle, NULL);

    if (m_TransHandle != Handle)
    {
        PRINT_LOG(ERROR, "This transaction is not attached to service get file");
        DUMP_VARIABLE(Handle);
        return;
    }

    m_TransHandle = GTP_INVALID_HANDLE;

    if (StateReceivingACK())
    {
        // Cancel task
        Application::GetSingleton().CancelDelayTask(ServiceGetFile::OnTimeProc, this);
    }

    // Switch service state to idle
    SwitchState(eIdle);

    // Close temp file if it is opened
    if (m_File != NULL)
    {
        fclose(m_File);
        m_File = NULL;
    }

    // Remove temp file
    unlink(TEMP_FILE_PATH);

    // Reset md5 engine
    m_MD5Engine.Reset();
}

void_t ServiceGetFile::OnTimeProc(void_t * Data)
{
    ASSERT_VALID_POINT(Data);
    ServiceGetFile * This = static_cast<ServiceGetFile *>(Data);
    This->OnTime();
}

void_t ServiceGetFile::OnTime()
{
    GMI_RESULT RetVal = GMI_SUCCESS;

    switch (m_State)
    {
    // If command canceled when packing file, then state will change to idle
    case eIdle:
    case ePackingFile:
        {
            // No thing to do
            break;
        }

    case eReceivingACK:
        {
            PRINT_LOG(WARNING, "Receive ACK timed out");
            // Send error response
            RetVal = SendSimpleResponse(m_TransHandle, "GetFile", GMI_WAIT_TIMEOUT);
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
            if (GTP_INVALID_HANDLE == m_TransHandle)
            {
                PRINT_LOG(WARNING, "No transaction is binding, no need to send response");
                return;
            }

            if (GMI_SUCCESS == ThreadRetVal)
            {
                // Send ACK
                RetVal = GtpTransSendData(m_TransHandle, l_XML_ACK, sizeof(l_XML_ACK), GTP_DATA_TYPE_UTF8_XML);
                if (GMI_SUCCESS == RetVal)
                {
                    SwitchState(eSendingACK);
                    return;
                }

                PRINT_LOG(ERROR, "Failed to send ACK");
                GtpDestroyTransHandle(m_TransHandle);
            }
            else
            {
                PRINT_LOG(ERROR, "Failed to run thread");
                DUMP_VARIABLE(ThreadRetVal);

                // Send error response
                RetVal = SendSimpleResponse(m_TransHandle, "GetFile", ThreadRetVal);
                if (GMI_SUCCESS == RetVal)
                {
                    SwitchState(eSendingResponse);
                    return;
                }

                PRINT_LOG(ERROR, "Failed to send response");
                GtpDestroyTransHandle(m_TransHandle);
            }
        }
        else
        {
            PRINT_LOG(ALERT, "Faild to stop thread");
        }
    }
    else
    {
        // Try again in 100ms
        RetVal = Application::GetSingleton().AddDelayTask(ServiceGetFile::OnTimeProc, this, 0, 100000);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to add delay task");

            // TODO:
        }
    }
}

GMI_RESULT ServiceGetFile::ThreadEntry()
{
    std::string CommandString;

    if (0 == m_FileList.size())
    {
        PRINT_LOG(WARNING, "No file need to be upload");
        return GMI_INVALID_OPERATION;
    }
    else if (1 == m_FileList.size())
    {
        m_Compressed = false;
        m_Encrypted = false;

        CommandString = "cp ";
        CommandString += m_FileList[0];
        CommandString += " ";
        CommandString += TEMP_FILE_PATH;
    }
    else
    {
        // TODO: Pack file list
        m_Compressed = true;
        m_Encrypted = false;

        return GMI_INVALID_OPERATION;
    }

    // Run command
    PRINT_LOG(VERBOSE, "Run command %s", CommandString.c_str());

    int32_t SysRet = system(CommandString.c_str());
    if (WIFEXITED(SysRet))
    {
        if (WEXITSTATUS(SysRet) != 0)
        {
            PRINT_LOG(ERROR, "Failed to run command %s", CommandString.c_str());
            DUMP_VARIABLE(WEXITSTATUS(SysRet));
            return GMI_SYSTEM_ERROR;
        }

        return GMI_SUCCESS;
    }
    else if (WIFSIGNALED(SysRet))
    {
            PRINT_LOG(ERROR, "Command %s is terminate by signal", CommandString.c_str());
            DUMP_VARIABLE(WTERMSIG(SysRet));
            return GMI_SYSTEM_ERROR;
    }

    PRINT_LOG(ALERT, "Unknown result of function 'system()'");
    DUMP_VARIABLE(SysRet);
    return GMI_SYSTEM_ERROR;
}

boolean_t ServiceGetFile::ParseACK(const uint8_t * Buffer, uint32_t BufferLength)
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
    if (NULL == Operation || strcmp(Operation, "GetFile") != 0)
    {
        PRINT_LOG(VERBOSE, "Not a get file ACK");
        return false;
    }

    PRINT_LOG(VERBOSE, "It is a get file ACK");
    return true;
}

GMI_RESULT ServiceGetFile::SendNextPacket()
{
    ASSERT_VALID_POINT(m_File);

    static uint8_t Buffer[1400];
    GMI_RESULT     RetVal  = GMI_SUCCESS;
    int32_t        ReadRet = fread(Buffer, 1, sizeof(Buffer), m_File);
    if (ReadRet > 0)
    {
        m_MD5Engine.Update(Buffer, ReadRet);
        return GtpTransSendData(m_TransHandle, Buffer, ReadRet, GTP_DATA_TYPE_BINARY);
    }
    else if (ReadRet < 0)
    {
        PRINT_LOG(ERROR, "Failed to read data from file");
        RetVal = SendSimpleResponse(m_TransHandle, "GetFile", GMI_SYSTEM_ERROR);
        if (GMI_SUCCESS == RetVal)
        {
            SwitchState(eSendingResponse);
        }
        else
        {
            PRINT_LOG(ERROR, "Failed to send response");
        }

        return RetVal;
    }

    m_MD5Engine.Finalize();
    m_MD5Engine.HexDigest(reinterpret_cast<char_t *>(Buffer), sizeof(Buffer));

    do
    {
        TiXmlDocument   XmlDoc;
        TiXmlPrinter    XmlPrinter;
        TiXmlElement  * XmlRootElement = NULL;
        TiXmlElement  * XmlFileInfo    = NULL;
        TiXmlElement  * XmlMd5         = NULL;
        TiXmlText     * XmlText        = NULL;

        // Create root element of response XML
        NEW_XML_ELEMENT(XmlRootElement, "Response", &XmlDoc, RetVal);
        XmlRootElement->SetAttribute("operation", "GetFile");
        XmlRootElement->SetAttribute("result", RetVal2Int(GMI_SUCCESS));

        // Create file information element
        NEW_XML_ELEMENT(XmlFileInfo, "FileInfo", XmlRootElement, RetVal);
        XmlFileInfo->SetAttribute("compress", m_Compressed ? 1 : 0);
        XmlFileInfo->SetAttribute("encrypted", m_Encrypted ? 1 : 0);

        // Create md5 element
        NEW_XML_ELEMENT(XmlMd5, "Md5", XmlFileInfo, RetVal);
        NEW_XML_TEXT(XmlText, reinterpret_cast<char_t *>(Buffer), XmlMd5, RetVal);

        // Convert xml document tree to string
        if (!XmlDoc.Accept(&XmlPrinter))
        {
            PRINT_LOG(ERROR, "Failed to generate response XML string");
            RetVal = GMI_FAIL;
            break;
        }

        RetVal = GtpTransSendData(m_TransHandle, reinterpret_cast<const uint8_t *>(XmlPrinter.CStr()), XmlPrinter.Size(), GTP_DATA_TYPE_UTF8_XML);
        if (GMI_SUCCESS == RetVal)
        {
            SwitchState(eSendingResponse);
        }
        else
        {
            PRINT_LOG(ERROR, "Failed to send response");
        }

        return RetVal;
    } while (0);

    RetVal = SendSimpleResponse(m_TransHandle, "GetFile", RetVal);
    if (GMI_SUCCESS == RetVal)
    {
        SwitchState(eSendingResponse);
    }
    else
    {
        PRINT_LOG(ERROR, "Failed to send response");
    }

    return RetVal;
}

