/******************************************************************************
modules		:    Daemon
name		:    gmi_hardware_monitor.c
function	:    monitor system hardware config . And config file load hardware driver
author		:    minchao.wang
version		:    1.0.0.0.0
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	7/10/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/

#include <linux/spi/spidev.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include "gmi_includes.h"
#include "gmi_hardware_monitor.h"
#include "gmi_config_api.h"
#include "gmi_debug.h"
#include "ipc_fw_v3.x_setting.h"
#include "ipc_fw_v3.x_resource.h"
#include "gmi_brdwrapper.h"
#include "sys_info_readonly.h"

static int32_t l_I2cFd = -1;
static int32_t l_SpiFd = -1;
static int32_t l_GpioFd = -1;

uint8_t BitRev8(uint8_t byte)
{
    return ByteRevTable[byte];
}

GMI_RESULT GMI_FileExists(const char_t *FileName)
{
    DAEMON_PRINT_LOG(INFO,"GMI_FileExists  Start!! ");
    if (access(FileName ,F_OK) == 0)
    {
        return GMI_SUCCESS;
    }
    else
    {
        return GMI_FAIL;
    }
}

GMI_RESULT GMI_FileOpen( const char_t *DevPath, int32_t *Fd)
{
    if (NULL == DevPath)
    {
        return GMI_INVALID_PARAMETER;
    }

    *Fd = open(DevPath, O_RDWR);
    if (*Fd < 0)
    {
        return GMI_FAIL;
    }

    return GMI_SUCCESS;

}

GMI_RESULT GMI_FileClose(int32_t Fd)
{
    if (Fd > 0)
    {
        close(Fd);
    }

    return GMI_SUCCESS;
}

GMI_RESULT GMI_GpioRead(int32_t Fd, uint32_t GpioId, uint32_t *Value)
{

    GpioControl l_GpioCotl;
    l_GpioCotl.id = GpioId;
    l_GpioCotl.data = 0;

    if (ioctl(Fd, AMBA_DEBUG_IOC_GET_GPIO, &l_GpioCotl) < 0)
    {
        DAEMON_PRINT_LOG(ERROR,"ioctl Cmd Error");
        return GMI_FAIL;
    }

    *Value = l_GpioCotl.data;

    return GMI_SUCCESS;
}

GMI_RESULT GMI_GpioWrite(int32_t Fd, uint32_t GpioId, uint32_t Value)
{
    GpioControl l_GpioCotl;
    l_GpioCotl.id = GpioId;
    l_GpioCotl.data = Value;

    if (ioctl(Fd, AMBA_DEBUG_IOC_SET_GPIO, &l_GpioCotl) < 0)
    {
        DAEMON_PRINT_LOG(ERROR,"ioctl Cmd Error");
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}

GMI_RESULT GMI_I2cWrite(int32_t Fd, uint8_t Addr, uint8_t *Buf, uint32_t Buflen)
{
    struct i2c_msg msg;
    struct i2c_rdwr_ioctl_data IoctlData;

    if (Fd < 0)
    {
        return GMI_FAIL;
    }

    msg.addr = Addr;
    msg.flags = 0;
    msg.len = Buflen;
    msg.buf = Buf;

    IoctlData.msgs = &msg;
    IoctlData.nmsgs = 1;

    if (ioctl(Fd, I2C_RDWR, &IoctlData) < 0)
    {
        DAEMON_PRINT_LOG(ERROR,"ioctl Cmd Error");
        return GMI_FAIL;
    }

    return GMI_SUCCESS;

}

GMI_RESULT GMI_I2cRead(int32_t Fd, uint8_t Addr, uint8_t *Buf, uint32_t Buflen)
{
    struct i2c_msg msg;
    struct i2c_rdwr_ioctl_data IoctlData;

    if (Fd < 0 || Buf == NULL)
    {
        return GMI_FAIL;
    }

    msg.addr = Addr;
    msg.flags = I2C_M_RD;
    msg.len = Buflen;
    msg.buf = Buf;

    IoctlData.msgs = &msg;
    IoctlData.nmsgs = 1;

    if (ioctl(Fd, I2C_RDWR, &IoctlData) < 0)
    {
        DAEMON_PRINT_LOG(ERROR,"ioctl Cmd Error");
        return GMI_FAIL;
    }

    return GMI_SUCCESS;

}

GMI_RESULT GMI_SpiTrans(int32_t Fd, uint32_t Len, uint8_t *Rbuf, uint8_t *Wbuf)
{
    struct spi_ioc_transfer xfer;

    if (Fd < 0)
    {
        DAEMON_PRINT_LOG(ERROR,"Ioctl Cmd failed");
        return GMI_FAIL;
    }

    memset(&xfer, 0, sizeof(xfer));
    xfer.rx_buf = (ulong_t)Rbuf;
    xfer.tx_buf = (ulong_t)Wbuf;
    xfer.len = Len;

    if (ioctl(Fd, SPI_IOC_MESSAGE(1), &xfer) < 0) {
        DAEMON_PRINT_LOG(ERROR,"Ioctl Cmd failed 0x%02x",Rbuf[2]);
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}


GMI_RESULT GMI_SpiSpecify(int32_t Fd, uint8_t Mode, uint32_t Lsb, uint8_t Bit, uint32_t Speed)
{

    if (Fd < 0)
    {
        return GMI_FAIL;
    }

    ioctl(Fd, SPI_IOC_WR_LSB_FIRST, &Lsb);
    ioctl(Fd, SPI_IOC_WR_BITS_PER_WORD, &Bit);
    ioctl(Fd, SPI_IOC_WR_MAX_SPEED_HZ, &Speed);
    ioctl(Fd, SPI_IOC_WR_MODE, &Mode);

    uint8_t mode = 0, lsb = 0, bits = 0;
    uint32_t speed = 0;


    GMI_RESULT Ret = -1;

    Ret =  ioctl(Fd, SPI_IOC_RD_MODE, &mode);
    if (Ret < 0) {
        DAEMON_PRINT_LOG(ERROR,"Get Spi mode failed");
        goto exit;
    }
    Ret =  ioctl(Fd, SPI_IOC_RD_LSB_FIRST, &lsb);
    if (Ret < 0) {
        DAEMON_PRINT_LOG(ERROR,"Get Spi lsb failed");
        goto exit;
    }
    Ret =  ioctl(Fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (Ret < 0) {
        DAEMON_PRINT_LOG(ERROR,"Get Spi bits failed");
        goto exit;
    }
    Ret =  ioctl(Fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (Ret < 0) {
        DAEMON_PRINT_LOG(ERROR,"Get Spi Speed failed");
        goto exit;
    }

    if (Mode!= mode || Lsb!= lsb || Bit != bits || Speed !=speed) {
        DAEMON_PRINT_LOG(ERROR,"read data is not equal set data");
        goto exit;
    }

    DAEMON_PRINT_LOG(INFO,"mode 0x%02x, lsb %d, bits %d, speed %d", mode, lsb, bits, speed);

    return GMI_SUCCESS;
exit:
    return GMI_FAIL;
}

GMI_RESULT GMI_Mn34041SensorCheck(void)
{
    /* send "0x82 0x1c" will read 0x1c register, it is fixed to 0x50 */
    GMI_RESULT Ret = GMI_FAIL;

    Ret = GMI_FileOpen(SPI_DEV_PATH, &l_SpiFd);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"Open SPI  Dev fail!!");
        return GMI_FAIL;
    }

    Ret = GMI_SpiSpecify(l_SpiFd, SPI_MODE_3|SPI_LSB_FIRST, 1, 8, 1000000);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_SpiSpecify fail!!!");
        GMI_FileClose(l_SpiFd);
        return GMI_FAIL;
    }

    uint8_t Cmd[4] = {0x36, 0x80, 0x00,0x00};
    uint8_t i = 0;
    /* amba's SPI don't support LSB_Fisrt, so we need to reserval the bytes	*/
    for (i=0; i < sizeof(Cmd); i++)
    {
        Cmd[i] = (uint8_t)BitRev8(Cmd[i]);
    }

    uint8_t Rbuf[4] = {0x00};

    Ret = GMI_SpiTrans(l_SpiFd, 4, Rbuf, Cmd);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"Commit with spi failed!!");
        GMI_FileClose(l_SpiFd);
        return GMI_FAIL;
    }

    for (i = 0; i < sizeof(Rbuf); i++)
        Rbuf[i] = BitRev8(Rbuf[i]);

    DAEMON_PRINT_LOG(INFO,"Imx Spi Read : 0x%02x 0x%02x 0x%02x 0x%02x", Rbuf[0], Rbuf[1], Rbuf[2], Rbuf[3]);

    if (Rbuf[3] != 0x30)
    {
        DAEMON_PRINT_LOG(ERROR,"Rbuf is not compete to register Rbuf[3] = 0x%02x!!",Rbuf[3]);
        GMI_FileClose(l_SpiFd);
        return GMI_FAIL;
    }

    GMI_FileClose(l_SpiFd);
    return GMI_SUCCESS;

}


GMI_RESULT GMI_Imx122SensorCheck(void)
{

    /* send "0x82 0x1c" will read 0x1c register, it is fixed to 0x50 */
    GMI_RESULT Ret = GMI_FAIL;

    Ret = GMI_FileOpen(SPI_DEV_PATH, &l_SpiFd);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"Open SPI  Dev fail!!");
        return GMI_FAIL;
    }

    Ret = GMI_SpiSpecify(l_SpiFd, SPI_MODE_3, 0, 8, 1000000);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_SpiSpecify fail!!!");
        GMI_FileClose(l_SpiFd);
        return GMI_FAIL;
    }

    uint8_t Cmd[3] = {0x82, 0x1c, 0x00};
    uint8_t i = 0;

    /* amba's SPI don't support LSB_Fisrt, so we need to reserval the bytes	*/
    for (i=0; i < sizeof(Cmd); i++)
    {
        Cmd[i] = BitRev8(Cmd[i]);
    }

    uint8_t Rbuf[3] = {0x00};

    Ret = GMI_SpiTrans(l_SpiFd, 3, Rbuf, Cmd);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"commit with spi failed!!!");
        GMI_FileClose(l_SpiFd);
        return GMI_FAIL;
    }

    for (i = 0; i < sizeof(Rbuf); i++)
    {
        Rbuf[i] = BitRev8(Rbuf[i]);
    }

    DAEMON_PRINT_LOG(INFO,"Imx Spi Read : 0x%02x 0x%02x 0x%02x", Rbuf[0], Rbuf[1], Rbuf[2]);

    if (Rbuf[2] != 0x50)
    {
        DAEMON_PRINT_LOG(ERROR,"Rbuf is not compete to register Rbuf[2] = 0x%02x!!",Rbuf[2]);
        GMI_FileClose(l_SpiFd);
        return GMI_FAIL;
    }

    GMI_FileClose(l_SpiFd);
    return GMI_SUCCESS;
}

GMI_RESULT GMI_Tw9910DecodeCheck(void)
{
    GMI_RESULT Ret = GMI_FAIL;

    Ret = GMI_FileOpen(I2C_DEV_PATH, &l_I2cFd);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"Open I2C  Dev fail!!");
        return GMI_FAIL;
    }

    uint8_t Wdata = 0x00;
    Ret = GMI_I2cWrite(l_I2cFd, TW9910_I2C_ADDR, &Wdata, 1);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR," I2C Send value Error!!\n");
        GMI_FileClose(l_I2cFd);
        return GMI_FAIL;
    }

    uint8_t ReadID = 0;
    Ret = GMI_I2cRead(l_I2cFd, TW9910_I2C_ADDR, &ReadID,  1);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR," I2C Read value Error!!\n");
        GMI_FileClose(l_I2cFd);
        return GMI_FAIL;
    }

    if ((ReadID & 0xf8) != 0x58 )
    {
        DAEMON_PRINT_LOG(ERROR," I2C Read value ReadID = 0x%02x!!\n",ReadID);
        GMI_FileClose(l_I2cFd);
        return GMI_FAIL;
    }

    GMI_FileClose(l_I2cFd);
    return GMI_SUCCESS;
}


GMI_RESULT GMI_Ov9715SensorCheck(void)
{
    GMI_RESULT Ret = GMI_FAIL;

    Ret = GMI_FileOpen(I2C_DEV_PATH, &l_I2cFd);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR," Open I2C  Dev fail!!\n!!\n");
        return GMI_FAIL;
    }

    uint8_t Wdata = 0x0a;
    Ret = GMI_I2cWrite(l_I2cFd, OV9715_I2C_ADDR, &Wdata, 1);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR," I2C Send value Error!!\n!!\n");
        GMI_FileClose(l_I2cFd);
        return GMI_FAIL;
    }

    uint8_t ReadIDH=0, ReadIDL=0;
    Ret = GMI_I2cRead(l_I2cFd, OV9715_I2C_ADDR, &ReadIDH,  1);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"  I2C Read value Error!!!!\n");
        GMI_FileClose(l_I2cFd);
        return GMI_FAIL;
    }

    Wdata=0x0b;
    Ret = GMI_I2cWrite(l_I2cFd, OV9715_I2C_ADDR, &Wdata, 1);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"  I2C Send value Error!!!!\n");
        GMI_FileClose(l_I2cFd);
        return GMI_FAIL;
    }

    Ret = GMI_I2cRead(l_I2cFd, OV9715_I2C_ADDR, &ReadIDL,  1);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"  I2C Read value Error!!!!\n");
        GMI_FileClose(l_I2cFd);
        return GMI_FAIL;
    }

    if ((ReadIDH != 0x97) || (ReadIDL != 0x11))
    {
        DAEMON_PRINT_LOG(ERROR,"  I2C Read value ReadIDH = 0x%02x ReadIDL = 0x%02x !!!!\n",ReadIDH,ReadIDL);
        GMI_FileClose(l_I2cFd);
        return GMI_FAIL;
    }

    GMI_FileClose(l_I2cFd);
    return GMI_SUCCESS;
}


GMI_RESULT GMI_Ov2715SensorCheck(void)
{
    GMI_RESULT Ret = GMI_FAIL;

    Ret = GMI_FileOpen(I2C_DEV_PATH, &l_I2cFd);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR," Open I2C  Dev fail!!\n!!\n");
        return GMI_FAIL;
    }

    uint8_t Wdata[2];
    Wdata[0]=0x30;
    Wdata[1]=0x0a;
    Ret = GMI_I2cWrite(l_I2cFd, OV2715_I2C_ADDR, Wdata, 2);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR," I2C Send value Error!!\n!!\n");
        GMI_FileClose(l_I2cFd);
        return GMI_FAIL;
    }

    uint8_t ReadIDH=0, ReadIDL=0;
    Ret = GMI_I2cRead(l_I2cFd, OV2715_I2C_ADDR, &ReadIDH,  1);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"  I2C Read value Error!!!!\n");
        GMI_FileClose(l_I2cFd);
        return GMI_FAIL;
    }

    Wdata[0]=0x30;
    Wdata[1]=0x0b;
    Ret = GMI_I2cWrite(l_I2cFd, OV2715_I2C_ADDR, Wdata, 2);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR," I2C Send value Error!!\n!!\n");
        GMI_FileClose(l_I2cFd);
        return GMI_FAIL;
    }

    Ret = GMI_I2cRead(l_I2cFd, OV2715_I2C_ADDR, &ReadIDL,  1);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"  I2C Read value Error!!!!\n");
        GMI_FileClose(l_I2cFd);
        return GMI_FAIL;
    }

    if ((ReadIDH != 0x27) || (ReadIDL != 0x10))
    {
        DAEMON_PRINT_LOG(ERROR,"  I2C Read value ReadIDH = 0x%02x ReadIDL = 0x%02x !!!!\n",ReadIDH,ReadIDL);
        GMI_FileClose(l_I2cFd);
        return GMI_FAIL;
    }

    GMI_FileClose(l_I2cFd);
    return GMI_SUCCESS;
}

GMI_RESULT GMI_CheckSensorType(HardwareConfig *Hardware)
{
    char_t CmdBuffer[MAX_BUFFER_LENGTH];

    DAEMON_PRINT_LOG(INFO,"GMI_CheckSensorType  Start!! ");

    memset(CmdBuffer, 0, sizeof(CmdBuffer));
    snprintf(CmdBuffer, 255,"%s",GMI_SENSOR_CHECK);
    if (system(CmdBuffer) < 0)
    {
        DAEMON_PRINT_LOG(ERROR," system running Error CmdBuffer = %sl!",CmdBuffer);
        return GMI_FAIL;
    }

    GMI_RESULT Ret = GMI_FAIL;

    Ret = GMI_Ov2715SensorCheck();
    if (SUCCEEDED(Ret))
    {
        DAEMON_PRINT_LOG(INFO," Check Sensor is ov2715l!");
        strcpy(Hardware->s_VideoIn,"ov2715");
        goto exit;
    }

    Ret = GMI_Ov9715SensorCheck();
    if (SUCCEEDED(Ret))
    {
        DAEMON_PRINT_LOG(INFO," Check Sensor is ov9715!");
        strcpy(Hardware->s_VideoIn,"ov9715");
        goto exit;
    }

    Ret = GMI_Imx122SensorCheck();
    if (SUCCEEDED(Ret))
    {
        DAEMON_PRINT_LOG(INFO," Check Sensor is imx122!");
        strcpy(Hardware->s_VideoIn,"imx122");
        goto exit;
    }

    Ret = GMI_Mn34041SensorCheck();
    if (SUCCEEDED(Ret))
    {
        DAEMON_PRINT_LOG(INFO," Check Sensor is mn34041pl!");
        strcpy(Hardware->s_VideoIn,"mn34041pl");
        goto exit;
    }

    Ret = GMI_Tw9910DecodeCheck();
    if (SUCCEEDED(Ret))
    {
        DAEMON_PRINT_LOG(INFO," Check Sensor is tw9910!");
        strcpy(Hardware->s_VideoIn,"tw9910");
        goto exit;
    }

    strcpy(Hardware->s_VideoIn,"");
    return  GMI_FAIL;

exit:
    memset(CmdBuffer, 0, sizeof(CmdBuffer));
    snprintf(CmdBuffer, 255,"rmmod ambdd");
    if (system(CmdBuffer) < 0)
    {
        DAEMON_PRINT_LOG(ERROR," system running Error CmdBuffer = %sl!",CmdBuffer);
        GMI_DeBugPrint("[%s][%d] System running Error  ! ",__func__,__LINE__);
        return GMI_FAIL;
    }

    return Ret;
}

GMI_RESULT GMI_LoadSensorDriver(const HardwareConfig *Hardware)
{
    if(NULL == Hardware)
    {
        return GMI_INVALID_PARAMETER;
    }

    char_t CmdBuffer[MAX_BUFFER_LENGTH];

    DAEMON_PRINT_LOG(INFO,"GMI_LoadSensorDriver  Start!! ");
    memset(CmdBuffer, 0, sizeof(CmdBuffer));
    snprintf(CmdBuffer, 255,"%s --%s",GMI_HARDWARE_CPU_CHECK, Hardware->s_VideoIn);
    if (system(CmdBuffer) < 0)
    {
        DAEMON_PRINT_LOG(ERROR," system running Error CmdBuffer = %sl!",CmdBuffer);
        GMI_DeBugPrint("[%s][%d]GMI_LoadSensorDriver %s ! ",__func__,__LINE__,Hardware->s_VideoIn);
        return GMI_FAIL;
    }

    sleep(2);
    return GMI_SUCCESS;
}


GMI_RESULT GMI_CheckCpuType(HardwareConfig *Hardware)
{
    GMI_RESULT Ret = GMI_FAIL;

    DAEMON_PRINT_LOG(INFO,"GMI_CheckCpuType  Start!! ");

    Ret = GMI_LoadSensorDriver(Hardware);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_LoadSensorDriver Error !");
        GMI_DeBugPrint("[%s][%d]GMI_LoadSensorDriver Error",__func__,__LINE__);
        return GMI_FAIL;
    }

    sleep(2);

    int32_t IavFd = -1;

    IavFd = open(AMBA_IAV_PATH, O_RDONLY);
    if (IavFd < 0)
    {
        DAEMON_PRINT_LOG(ERROR,"Open  Dev file Error !");
        GMI_DeBugPrint("[%s][%d]Open  Dev file Error ",__func__,__LINE__);

        return GMI_FAIL;
    }

    uint32_t ChipId = -1;

    if (ioctl(IavFd, IAV_IOC_GET_CHIP_ID_EX, &ChipId) < 0)
    {
        DAEMON_PRINT_LOG(ERROR,"Ioctl  Cmd file Error !");
        GMI_DeBugPrint("[%s][%d]Ioctl  Cmd file Error ! ",__func__,__LINE__);

        return GMI_FAIL;
    }

    switch (ChipId)
    {
    case IAV_CHIP_ID_A5S_33:
        DAEMON_PRINT_LOG(INFO,"CPU is A5S_33 !");
        strcpy(Hardware->s_CPU,"A5S_33");
        break;
    case IAV_CHIP_ID_A5S_55:
        DAEMON_PRINT_LOG(INFO,"CPU is A5S_55 !");
        strcpy(Hardware->s_CPU,"A5S_55");
        break;
    case IAV_CHIP_ID_A5S_66:
        DAEMON_PRINT_LOG(INFO,"CPU is A5S_66 !");
        strcpy(Hardware->s_CPU,"A5S_66");
        break;
    case IAV_CHIP_ID_A5S_88:
        DAEMON_PRINT_LOG(INFO,"CPU is A5S_88 !");
        strcpy(Hardware->s_CPU,"A5S_88");
        break;
    default:
        strcpy(Hardware->s_CPU,"");
        break;
    }

    close(IavFd);

    return GMI_SUCCESS;
}

GMI_RESULT GMI_CheckBoardType(HardwareConfig *Hardware)
{
    GMI_RESULT Ret = GMI_FAIL;
    uint32_t Value;

    DAEMON_PRINT_LOG(INFO,"GMI_CheckBoardType  Start!! ");

    Ret = GMI_FileOpen(AMBA_GPIO_PATH, &l_GpioFd);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"Open  Dev file Error !");
        GMI_DeBugPrint("[%s][%d] Open  Dev file Error  ! ",__func__,__LINE__);
        return Ret;
    }

    Ret = GMI_GpioRead(l_GpioFd, ENCODE_BOARD_GPIO_NUM,&Value);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_GpioRead  Error !");
        GMI_DeBugPrint("[%s][%d] GMI_GpioRead Error  ! ",__func__,__LINE__);
        GMI_FileClose(l_GpioFd);
        return Ret;
    }

    if (ENCODE_BOARD_LARK == Value)
    {
        DAEMON_PRINT_LOG(INFO,"BOARD  is LARK !");
        strcpy(Hardware->s_MainBoard,"LARK");
    }
    else if (ENCODE_BOARD_NORMAL== Value)
    {
        DAEMON_PRINT_LOG(INFO,"BOARD  is NORMAL !");
        strcpy(Hardware->s_MainBoard,"NORMAL");
    }
    else
    {
        DAEMON_PRINT_LOG(INFO,"BOARD  is NONE !");
        strcpy(Hardware->s_Len,"NONE");
    }

    GMI_FileClose(l_GpioFd);

    DAEMON_PRINT_LOG(INFO,"GMI_CheckBoardType  Success!! ");

    return Ret;
}

GMI_RESULT GMI_CheckLenType(HardwareConfig *Hardware)
{
    GMI_RESULT Ret = GMI_FAIL;
    uint32_t l_Value;
    uint32_t l_Value2;
    int32_t flags = 0;
    DAEMON_PRINT_LOG(INFO,"GMI_CheckLenType  Start!! ");

    Ret = GMI_FileOpen(AMBA_GPIO_PATH, &l_GpioFd);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"Open  Dev file Error !");
        GMI_DeBugPrint("[%s][%d] Open  Dev file Error  ! ",__func__,__LINE__);
        return Ret;
    }

    Ret = GMI_GpioRead(l_GpioFd, ZOOM_LENS_DF003_GPIO, &l_Value);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_GpioRead  Error !");
        GMI_DeBugPrint("[%s][%d] GMI_GpioReadError  ! ",__func__,__LINE__);
        GMI_FileClose(l_GpioFd);
        return Ret;
    }

    Ret = GMI_GpioRead(l_GpioFd, ZOOM_LENS_YB22_GPIO, &l_Value2);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_GpioRead  Error !");
        GMI_DeBugPrint("[%s][%d] GMI_GpioReadError  ! ",__func__,__LINE__);
        GMI_FileClose(l_GpioFd);
        return Ret;
    }

    if (0 == strcmp(Hardware->s_MainBoard,"LARK"))
    {
        flags = 1;
    }

    if ((1 == l_Value) && (0 == l_Value2) && (1 == flags))
    {
        DAEMON_PRINT_LOG(INFO,"LENS  is DF003 !");
        strcpy(Hardware->s_Len,"DF003");
    }
    else if ((0==l_Value) && (1 ==l_Value2))
    {
        DAEMON_PRINT_LOG(INFO,"LENS  is YB22 !");
        strcpy(Hardware->s_Len,"YB22");
    }
    else
    {
        DAEMON_PRINT_LOG(INFO,"LENS  is NONE !");
        strcpy(Hardware->s_Len,"NONE");
    }
    GMI_FileClose(l_GpioFd);

    return Ret;

}


GMI_RESULT GMI_HardWareMonitor(HardwareConfig *Hardware)
{
    GMI_RESULT Ret = GMI_FAIL;

    DAEMON_PRINT_LOG(INFO,"GMI_HardWareMonitor  Start!! ");

    sleep(2);

    Ret = GMI_CheckBoardType(Hardware);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_CheckBoardType is Fail !");
        GMI_DeBugPrint("[%s][%d]GMI_CheckBoardType Error ! ",__func__,__LINE__);

        return Ret;
    }

    Ret = GMI_CheckLenType(Hardware);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_CheckBoardType is Fail !");
        GMI_DeBugPrint("[%s][%d]GMI_CheckLenType Error ! ",__func__,__LINE__);

        return Ret;
    }

    Ret = GMI_CheckSensorType(Hardware);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_CheckBoardType is Fail !");
        GMI_DeBugPrint("[%s][%d]GMI_CheckSensorType Error ! ",__func__,__LINE__);

        return Ret;
    }

    Ret = GMI_CheckCpuType(Hardware);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_CheckBoardType is Fail !");
        GMI_DeBugPrint("[%s][%d]GMI_CheckCpuType Error ! ",__func__,__LINE__);

        return Ret;
    }

    DAEMON_PRINT_LOG(INFO,"GMI_HardWareMonitor  GMI_SUCCESS!! ");

    return GMI_SUCCESS;
}

GMI_RESULT GMI_SetHardwareConfig(const char_t * FileName, const char_t *ItemPath, HardwareConfig *Hardware)
{
    if (ItemPath == NULL || FileName == NULL )
    {
        DAEMON_PRINT_LOG(ERROR,"Invalid param !");
        return GMI_INVALID_PARAMETER;
    }

    DAEMON_PRINT_LOG(INFO,"GMI_SetHardwareConfig  Start!! ");

    GMI_RESULT Ret = GMI_FAIL;
    FD_HANDLE  Handle = NULL;

    Ret = GMI_XmlOpen(FileName, &Handle);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_XmlOpen xml file Error!");
        return Ret;
    }
    char_t  Key[MAX_BUFFER_LENGTH]= {"0"};

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "CPU");
    Ret = GMI_XmlWrite(Handle, ItemPath, Key, Hardware->s_CPU);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_XmlWrite  Error!");
        return Ret;
    }

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "VideoIn");
    Ret = GMI_XmlWrite(Handle, ItemPath, Key, Hardware->s_VideoIn);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_XmlWrite  Error!");
        return Ret;
    }

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "MainBoard");
    Ret = GMI_XmlWrite(Handle, ItemPath, Key, Hardware->s_MainBoard);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_XmlWrite	Error!");
        return Ret;
    }

    memset(Key, 0 ,sizeof(Key));
    strcpy(Key, "Lens");
    Ret = GMI_XmlWrite(Handle, ItemPath, Key, Hardware->s_Len);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_XmlWrite  Error!");
        return Ret;
    }

    Ret = GMI_XmlFileSave(Handle);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(ERROR,"GMI_XmlFileSave xml file Error");
        return Ret;
    }

    return GMI_SUCCESS;
}

GMI_RESULT GMI_GetHardwareConfig(const char_t * FileName, const char_t *ItemPath, HardwareConfig *Hardware)
{
    if (ItemPath == NULL || FileName == NULL )
    {
        DAEMON_PRINT_LOG(ERROR,"Invalid param !");
        return GMI_INVALID_PARAMETER;
    }

    GMI_RESULT Result = SysInfoReadInitialize();
    if (FAILED(Result))
    {
        return Result;
    }

    FD_HANDLE Handle;
    Result = SysInfoOpen(FileName, &Handle);
    if (FAILED(Result))
    {
        SysInfoReadDeinitialize();
        return Result;
    }

    DAEMON_PRINT_LOG(INFO,"GMI_GetHardwareConfig  Start!! ");
    char_t  Value[MAX_BUFFER_LENGTH] = {"0"};
    memset(Value, 0 ,sizeof(Value));
    Result = SysInfoRead(Handle, ItemPath, HW_CPU_KEY, "A5S_33", Value);
    if (FAILED(Result))
    {
        SysInfoClose(Handle);
        SysInfoReadDeinitialize();
        return Result;
    }
    else
    {
        strcpy(Hardware->s_CPU, Value);
    }

    memset(Value, 0 ,sizeof(Value));
    Result = SysInfoRead(Handle, ItemPath, HW_SENSOR_KEY, HW_SENSOR, Value);
    if (FAILED(Result))
    {
        SysInfoClose(Handle);
        SysInfoReadDeinitialize();
        return Result;
    }
    else
    {
        strcpy(Hardware->s_VideoIn, Value);
    }

    memset(Value, 0 ,sizeof(Value));
    Result = SysInfoRead(Handle, ItemPath, HW_MAINBOARD_KEY, HW_MAINBOARD, Value);
    if (FAILED(Result))
    {
        SysInfoClose(Handle);
        SysInfoReadDeinitialize();
        return Result;
    }
    else
    {
        strcpy(Hardware->s_MainBoard, Value);
    }

    memset(Value, 0 ,sizeof(Value));
    Result = SysInfoRead(Handle, ItemPath, HW_LENS_KEY, HW_LENS, Value);
    if (FAILED(Result))
    {
        SysInfoClose(Handle);
        SysInfoReadDeinitialize();
        return Result;
    }
    else
    {
        strcpy(Hardware->s_Len, Value);
    }

    SysInfoClose(Handle);
    SysInfoReadDeinitialize();

    return Result;
}

/*=======================================================
name				:	GMI_GetUpdateServerPort
function			:  Get System Update Server Port
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	10/24/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_GetUpdateServerPort(int32_t *Port)
{
    GMI_RESULT Result = SysInfoReadInitialize();
    if (FAILED(Result))
    {
        return Result;
    }

    FD_HANDLE Handle;
    Result = SysInfoOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if (FAILED(Result))
    {
        SysInfoReadDeinitialize();
        return Result;
    }

    Result = SysInfoRead(Handle, GMI_EXTERN_NETWORK_PORT_PATH, GMI_DAEMON_UPDATE_SERVER_PORT_KEY, GMI_DAEMON_UPDATE_SERVER_PORT, Port);
    if (FAILED(Result))
    {
        SysInfoClose(Handle);
        SysInfoReadDeinitialize();
        return Result;
    }

    SysInfoClose(Handle);
    SysInfoReadDeinitialize();

    return GMI_SUCCESS;
}

GMI_RESULT GMI_CheckCpu(const HardwareConfig *Hardware)
{
    const char_t Cpu[16] = "A5S_33";

    if(0 == strcmp(Cpu, Hardware->s_CPU))
    {
        GMI_DeBugPrint("[%s][%d] CPU check  Error ! ",__func__,__LINE__);
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}

/*=============================================================================
name				:	GMI_HardwareInit
function			:  Init System Hardware load sensor driver
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    no
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	7/10/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_HardwareInit(void)
{
    GMI_RESULT Ret = GMI_FAIL;
    char_t CmdBuffer[MAX_BUFFER_LENGTH];

    //Is file exists. If config.xml file exists don't do hardware monitor
    Ret = GMI_FileExists(GMI_HARDWARE_CONFIG_FILE);
    if (FAILED(Ret))
    {
        DAEMON_PRINT_LOG(INFO,"Config File is not Exists!! ");
        //cpu_check_bak running for cpu check
        Ret = GMI_FileExists("/opt/bin/cpu_check_bak");
        if (SUCCEEDED(Ret))
        {
            memset(CmdBuffer, 0, sizeof(CmdBuffer));
            snprintf(CmdBuffer, 255,"mv	/opt/bin/cpu_check_bak  /opt/bin/cpu_check");
            if (system(CmdBuffer) < 0)
            {
                DAEMON_PRINT_LOG(ERROR,"System running is Error CmdBuffer = %s!! ",CmdBuffer);
                return GMI_FAIL;
            }
        }

        //hardware recognition
        Ret = GMI_HardWareMonitor(g_Hardware);
        if (SUCCEEDED(Ret))
        {
            Ret = GMI_SetHardwareConfig(GMI_HARDWARE_CONFIG_FILE,GMI_HARDWARE_PATH,g_Hardware);
            if (SUCCEEDED(Ret))
            {
                GMI_DeBugPrint("[%s][%d]GMI_SetHardwareConfig Error ! ",__func__,__LINE__);
                Ret = GMI_BrdHwReset();
                if (FAILED(Ret))
                {
                    DAEMON_PRINT_LOG(ERROR,"GMI_Reboot! !!!");
                }
            }
        }
        else
        {
            GMI_DeBugPrint("[%s][%d]GMI_HardWareMonitor Error ! ",__func__,__LINE__);
        }
    }
    else if (SUCCEEDED(Ret))
    {
        DAEMON_PRINT_LOG(INFO,"Config File is  Exists, Get System hardware config!! ");
        //Get Hardware config
        Ret = GMI_GetHardwareConfig(GMI_HARDWARE_CONFIG_FILE, GMI_HARDWARE_PATH, g_Hardware);
        if (FAILED(Ret))
        {
            DAEMON_PRINT_LOG(ERROR,"GMI_GetHardwareConfig Error!! ");
            return Ret;
        }

        Ret = GMI_CheckCpu(g_Hardware);
        if (FAILED(Ret))
        {
            sleep(10);
            memset(CmdBuffer, 0, sizeof(CmdBuffer));
            snprintf(CmdBuffer, 255,"rm -rf %s",GMI_HARDWARE_CONFIG_FILE);
            if (system(CmdBuffer) < 0)
            {
                DAEMON_PRINT_LOG(ERROR,"System running is Error CmdBuffer = %s!! ",CmdBuffer);
                return GMI_FAIL;
            }

            memset(CmdBuffer, 0, sizeof(CmdBuffer));
            snprintf(CmdBuffer, 255,"mv  /opt/bin/cpu_check_bak  /opt/bin/cpu_check");
            if (system(CmdBuffer) < 0)
            {
                DAEMON_PRINT_LOG(ERROR,"System running is Error CmdBuffer = %s!! ",CmdBuffer);
                return GMI_FAIL;
            }

            Ret = GMI_BrdHwReset();
            if (FAILED(Ret))
            {
                DAEMON_PRINT_LOG(ERROR,"GMI_Reboot! !!!");
            }
            return Ret;
        }

        //Running load sensor dirver
        Ret = GMI_LoadSensorDriver(g_Hardware);
        if (FAILED(Ret))
        {
            DAEMON_PRINT_LOG(ERROR,"GMI_LoadSensorDriver Error!! ");
            return Ret;
        }
    }

    Ret = GMI_GetUpdateServerPort(&g_UpdatePort);
    if (FAILED(Ret))
    {
        g_UpdatePort = GMI_DAEMON_UPDATE_SERVER_PORT;
        GMI_DeBugPrint("[%s][%d]GMI_GetUpdateServerPort fail, Default UpdatePort = [%d]! ", __func__, __LINE__, g_UpdatePort);
        // return Ret;
    }

    return Ret;
}



