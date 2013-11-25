/******************************************************************************
modules		:    BoardWrapper
name		:    brdwrapperdef.h
function	:    The public of macro definition and struct data for  the user of bscr driver and  application.
			the main of funciton this modules for monitor card device
author		:    minchao.wang
version		:    1.0.0.0.0
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	6/1/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/

#ifndef __GMI_BRD_WRAPPER_DEF_H
#define __GMI_BRD_WRAPPER_DEF_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define PTZ_DEVICE_PATH         "/dev/ttyS1"

//Alarm Port mode
#define GMI_ALARM_MODE_GPIO    0
#define GMI_ALARM_MODE_I2C    1
#define GMI_ALARM_MODE_UART    2
#define GMI_ALARM_MODE_UART_DN    3

//Alarm in Status
#define GMI_ALARM_IN_STATUS_HIGHT     1
#define GMI_ALARM_IN_STATUS_LOW        0
#define GMI_ALARM_IN_UART_DAY           0
#define GMI_ALARM_IN_UART_NIGHT        1

//Borad ID define
//the undefine borad ID
#define UNKNOWN_BOARD               0xff
//the IPC borad ID
#define BRD_GMIPC_2000                 0x01

//Led running macro definition
//the max number of LED
#define BRD_LED_NUM_MAX             32
//led status on
#define BRD_LED_ON                  1
//led status off
#define BRD_LED_OFF                 2
//led  quickly running    (0.25s  ON --->0.25s OFF ---> 0.25s ON ---)
#define BRD_LED_QUICK               3
//led  slow running        (1s ON --->1s OFF --->1s ON---)
#define BRD_LED_SLOW                4
//led  standby running   (2s ON--->2s OFF--->2s ON ---)
#define BRD_LED_STANDBY             5

//BrdLedStatusSet  function led control  ID
//the led for system running
#define LED_SYS_RUN                 0xe0
//the led for application  running
#define LED_APP_RUN                 0xe1

//RTC  ship type define
#define RTC_TYPE_NONE               0
#define RTC_TYPE_SOC             	1
#define RTC_TYPE_DS1302		2

// function return value define
//#define OK                          0
//#define ERROR                       -1
//#define STATUS                      int

//Day Mode Or colour Mode
#define DEVICE_DAY_MODE	1     
//Night Mode Or Black Mode
#define DEVICE_NIGHT_MODE  0

//define   the  macro  for system
//the lenght of name
#define INTERFACE_NAME_MAX_LEN      10
//the lenght of serial name
#define TTY_NAME_MAX_LEN            20
//the max of common string
#define STR_NAME_MAX_LEN            16
//the max of mac number
#define ETH_MAC_MAX_NUM             8

//define the  macro for  wacthdog
//the clk of feed hardware watchdog
#define WATCHDOG_USE_CLK            0x00
//soft feed watchdog
#define WATCHDOG_USE_SOFT           0x01
//stop feed watchdog
#define WATCHDOG_STOP               0x02

//define  the  macro for  alarm
//define  the max of alarm num for system
#define BRD_ALM_NUM_MAX             256

//define the  macro for seriral

#define BRD_SERIAL_RS485_RECEIVE  0
#define BRD_SERIAL_RS485_SEND    1
// BrdOpenSerial  function  open serial type
#define BRD_SERIAL_RS232             0
#define BRD_SERIAL_RS422             1
#define BRD_SERIAL_RS485             2
#define BRD_SERIAL_INFRARED          3

//define the macro for serial 485
//open success
#define RS485_SUCCESS                     0
//485 is not open
#define RS485_NOT_OPENED                  1
//485 device is not connected
#define RS485_NOT_CONNECTED               2
//send timeout
#define RS485_SND_TIMEOUT                 3
//recevie timeout
#define RS485_RCV_TIMEOUT                 4
//recevie data len error
#define RS485_RCV_LENERR                  5
//recevie data error
#define RS485_RCV_ERRDATA                 6
//operation error
#define RS485_ERROR                       -1

//set 485 send timeout value
#define RS485_SET_SND_TIMEOUT   4
//get 485 send timeout value
#define RS485_GET_SND_TIMEOUT   5

//define the macro for com serial
//set baudrate
#define SIO_SET_BAUDRATE        0x2000
//get baudrate
#define SIO_GET_BAUDRATE        0x2001
//set stop bit
#define SIO_SET_STOPBIT         0x2002
//get stop bit
#define SIO_GET_STOPBIT         0x2003
//set serial data bit
#define SIO_SET_DATABIT         0x2004
//get serial data bit
#define SIO_GET_DATABIT         0x2005
//set parity
#define SIO_SET_PARITY          0x2006
//get parity
#define SIO_GET_PARITY          0x2007
//set send timeout value
#define SIO_485_SET_SND_TIMEOUT 0x2008
//get send timeout value
#define SIO_485_GET_SND_TIMEOUT 0x2009

//serial parity none
#define SIO_PARITY_NONE       0
//serial parity odd
#define SIO_PARITY_ODD        1
//serial parity even
#define SIO_PARITY_EVEN       2
//serial parity mark
#define SIO_PARITY_MARK       3
//serial parity space
#define SIO_PARITY_SPACE      4
//serial stop bit 1
#define SIO_STOPBIT_1         0
//serial stop bit 2
#define SIO_STOPBIT_2         1

// System zone config
#define RTC_SHANGHAI_ZONE     "Shanghai"
#define RTC_UTC_ZONE                "UTC"

//Ptz Control Mode
#define  PTZ_CONTINUE_CONTROL_MODE     0
#define  PTZ_DISCONTINUE_CONTROL_MODE     1

//Ptz Control  Protocol
#define  PTZ_PROTOCOL_D_PROTOCOL        1
#define  PTZ_PROTOCOL_P_PROTOCOL         2
#define  PTZ_PROTOCOL_AIP_STAR_PTZ      3

// ctr cmd:
// 0-up, 1-down, 2-up stop, 3-down stop, 
// 4-left, 5-right, 6-left stop, 7-right stop
// 8-ZoomTele, 9-ZoomWide, 10-ZoomTele stop, 11-ZoomWide stop
// 12-FocusNear, 13-FocusFar, 14-FocusNear stop, 15-FocusFar stop
// 16-IrisOpen, 17-IrisClose, 18-IrisOpen stop, 19-IrisClose stop
// 20-Stop all
typedef enum{
    e_PTZ_CMD_Up=0,
    e_PTZ_CMD_Down,
    e_PTZ_CMD_UpStop,
    e_PTZ_CMD_DownStop,
    e_PTZ_CMD_Left,
    e_PTZ_CMD_Right,
    e_PTZ_CMD_LeftStop,
    e_PTZ_CMD_RightStop,
    e_PTZ_CMD_ZoomTele,
    e_PTZ_CMD_ZoomWide,
    e_PTZ_CMD_ZoomTeleStop,	// 10
    e_PTZ_CMD_ZoomWideStop,
    e_PTZ_CMD_FocusNear,
    e_PTZ_CMD_FocusFar,
    e_PTZ_CMD_FocusNearStop,
    e_PTZ_CMD_FocusFarStop,
    e_PTZ_CMD_IrisOpen,
    e_PTZ_CMD_IrisClose,
    e_PTZ_CMD_IrisOpenStop,
    e_PTZ_CMD_IrisCloseStop,
    e_PTZ_CMD_SetSpeed,	// 20
    
    e_PTZ_CMD_SetPreset,
    e_PTZ_CMD_RmPreset,
    e_PTZ_CMD_GotoPreset,
    
    e_PTZ_CMD_Stop,
    
    e_PTZ_CMD_LeftUp, //25
    e_PTZ_CMD_LeftDown,
    e_PTZ_CMD_RightUp,
    e_PTZ_CMD_RightDown,
    
    e_PTZ_CMD_LeftLimit, // 29 this is compare with define in UART_EXT_CMD_LEFT_LIMIT
    e_PTZ_CMD_RightLimit,
    e_PTZ_CMD_UpLimit,
    e_PTZ_CMD_DownLimit,
    e_PTZ_CMD_ScanStart,
    e_PTZ_CMD_ScanStop,
    e_PTZ_CMD_AuxpntOpen,
    e_PTZ_CMD_AuxpntClose, // 36 this is compare with UART_EXT_CMD_AUXPNT_CLOSE
    e_PTZ_CMD_FocusNearClick,
    e_PTZ_CMD_FocusFarClick,
    e_PTZ_CMD_H_360,
    
    e_PTZ_CMD_Invalid = 999,
}PtzCmd;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BRD_WRAPPER_DEF_H */

