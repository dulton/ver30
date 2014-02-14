#ifndef __GMI_MEDIA_CTRL_H__
#define __GMI_MEDIA_CTRL_H__

#include "gmi_system_headers.h"

#ifdef __cplusplus
extern "C"
{
#endif

//media type
    typedef enum 
    {
        MEDIA_VIDEO = 1,
        MEDIA_AUDIO,
    } MediaType;

//codec type
    typedef enum 
    {
        CODEC_H264 = 1,
        CODEC_MPEG4,
        CODEC_MJPEG,
        CODEC_G711A,
        CODEC_G711U,
        CODEC_G726,
    } CodecType;

//frame type
    typedef enum 
    {
        VIDEO_NA_FRAME  = -1,   /**< Frame type not available. */
        VIDEO_I_FRAME   = 2,    /**< Intra coded frame. */
        VIDEO_P_FRAME   = 3,    /**< Forward inter coded frame. */
        VIDEO_B_FRAME   = 4,    /**< Bi-directional inter coded frame. */
        VIDEO_IDR_FRAME = 1     /**< Intra coded frame that can be used for
                                                   *   refreshing video content */
    } VideoFrameType;

//bitrate ctrl mode
    typedef enum 
    {
        BIT_CBR = 1,
        BIT_VBR,
        BIT_CBRQ,
        BIT_VBRQ,
        BIT_CVBR
    } VideoBitCtrlMode;

//zoom control
    typedef enum
	{
		ZOOM_MODE_STOP = 0,    
        ZOOM_MODE_IN,          //enlarge object
        ZOOM_MODE_OUT
	}ZoomCtrlMode;

//af control
    typedef enum
	{
		AF_DIR_MODE_STOP = 0,    
        AF_DIR_MODE_IN,      //zoom in
        AF_DIR_MODE_OUT
	}AfCtrlDirMode;


//AF mode
	typedef enum
	{
		AF_MODE_MANUAL = 0,    
        AF_MODE_AUTO,
        AF_MODE_ONCEAUTO
	}AFCtrlMode;

    typedef struct
    {
        uint32_t  s_MaxBitRate;
        uint32_t  s_MinBitRate;
        uint32_t  s_AvgBitRate;
    } CVBR_Param;

//3D
	typedef enum
	{
		E3D_CMD_BOX_UNKNOWN = -1,
		E3D_CMD_BOX_CLICK = 0,		//box/click command
		E3D_CMD_MOUSE_DRAG,		    //toystick
		E3D_CMD_ZOOM_REL,		    //relative zoom
		E3d_CMD_EVENT_MAX = 2,		//
	}E3dPosCmdIdx;

	typedef enum
	{
		ERELZOOM_UNKNOWN = -1,
		ERELZOOM_NO = 0,
		ERELZOOM_ZOOMOUT = 1,	   //suo xiao
		ERELZOOM_ZOOMIN = 2,	   //fang da
		ERELZOOM_MAX = 2,
	} ERelZoomIdx;

//General Parameter
	typedef enum
	{
		GEN_PARAM_INVALID = -1,
		GEN_PARAM_AUTO	  = 0,				  //capability_auto.xml parameter
		GEN_PARAM_IRCUT 					  //gmi_factory_setting.xml ircut parameter
	}General_Param;

	// Define CPU type
	typedef enum 
	{
		E_CPU_UNKNOWN = 0,
		E_CPU_A5S_55,
		E_CPU_A5S_66,
		E_CPU_A5S_88		
	} SysCpuIdType;

	// Define vin type, include sensor and decoder
	typedef enum 
	{
		E_VIN_UNKNOWN = 0,
		E_SENSOR_MN34041,
		E_SENSOR_IMX122,
		E_SENSOR_OV2715,
		E_SENSOR_OV9715,
		E_DECODER_TW9910,
	} SysVinIdType;

	// Define encoder board type
	typedef enum 
	{
		E_ENCODER_BOARD_UNKNOWN = 0,
		E_ENCODER_BOARD_NORMAL,
		E_ENCODER_BOARD_LARK,
	} SysEncoderBoardIdType;

	// Define lens type
	typedef enum 
	{
		E_ZOOM_LENS_NONE = 0,
		E_ZOOM_LENS_DF003,
		E_ZOOM_LENS_YB22,
	} SysLensIdType;
//


	#define MAKE_MEDIA(m, c) ((m<<8)+c)

	#define MEDIA_VIDEO_H264 	  MAKE_MEDIA(MEDIA_VIDEO, CODEC_H264)
	#define MEDIA_VIDEO_MJPEG   MAKE_MEDIA(MEDIA_VIDEO, CODEC_MJPEG)
	#define MEDIA_VIDEO_MPEG4   MAKE_MEDIA(MEDIA_VIDEO, CODEC_MPEG4)
	#define MEDIA_AUDIO_G711A   MAKE_MEDIA(MEDIA_AUDIO, CODEC_G711A)
	#define MEDIA_AUDIO_G711U   MAKE_MEDIA(MEDIA_AUDIO, CODEC_G711U)
	#define MEDIA_AUDIO_G726    MAKE_MEDIA(MEDIA_AUDIO, CODEC_G726)

	//MediaId = 0 : main stream
	//MediaId = 1 : first substream
	//MediaId = 2 : sencod substream
	//MediaId = 3 : third substream
	typedef uint16_t MediaId;
	typedef uint16_t SensorId;

	//max video stream num
	#define MAX_VIDEO_STREAM_NUM   4
	#define OSD_TEXT_NUM            4

	//paramter range,type of []
	//video encode
	#define MIN_VIDEO_ENC_STREAMID           0
	#define MAX_VIDEO_ENC_STREAMID           3
	#define MIN_VIDEO_ENC_TYPE                1
	#define MAX_VIDEO_ENC_TYPE                2
	#define MIN_VIDEO_ENC_FRAMERATE          1
	#define MAX_VIDEO_ENC_FRAMERATE          60
	#define MIN_VIDEO_ENC_BITRATETYPE        1
	#define MAX_VIDEO_ENC_BITRATETYPE        2
	#define MIN_VIDEO_ENC_BITRATEAVG         32
	#define MAX_VIDEO_ENC_BITRATEAVG         32768     //32*1024          
	#define MIN_VIDEO_ENC_BITRATEUP          32
	#define MAX_VIDEO_ENC_BITRATEUP          32768
	#define MIN_VIDEO_ENC_BITRATEDOWN        32
	#define MAX_VIDEO_ENC_BITRATEDOWN        32768
	#define MIN_VIDEO_ENC_GOP                 1
	#define MAX_VIDEO_ENC_GOP                 400
	#define MIN_VIDEO_ENC_QULITY              1
	#define MAX_VIDEO_ENC_QULITY              100
	#define MIN_VIDEO_ENC_ROTATE              0
	#define MAX_VIDEO_ENC_ROTATE              3

	//VIN/VOUT
	#define MIN_VIDEO_IN_FLAG                 0
	#define MAX_VIDEO_IN_FLAG                 1
	#define MIN_VIDEO_IN_FRAMERATE           1
	#define MAX_VIDEO_IN_FRAMERATE           60
	#define MIN_VIDEO_IN_MIRRORMODE          0
	#define MAX_VIDEO_IN_MIRRORMODE          4
	#define MIN_VIDEO_IN_BAYERMODE           0
	#define MAX_VIDEO_IN_BAYERMODE           4

	#define MIN_VIDEO_OUT_TYPE               0
	#define MAX_VIDEO_OUT_TYPE               6
	#define MIN_VIDEO_OUT_FLIP               0
	#define MAX_VIDEO_OUT_FLIP               3

	//OSD
	#define MIN_VIDEO_OSD_STREAMID           0
	#define MAX_VIDEO_OSD_STREAMID           3
	#define MIN_VIDEO_OSD_DISTYPE            0
	#define MAX_VIDEO_OSD_DISTYPE            3
	#define MIN_VIDEO_OSD_BMPFLAG            0
	#define MAX_VIDEO_OSD_BMPFLAG            1
	#define MIN_VIDEO_OSD_LANGUAGE           0
	#define MAX_VIDEO_OSD_LANGUAGE           3

	#define MIN_VIDEO_OSD_TIMEFLAG           0
	#define MAX_VIDEO_OSD_TIMEFLAG           1
	#define MIN_VIDEO_OSD_DATASTYLE          0
	#define MAX_VIDEO_OSD_DATASTYLE          1
	#define MIN_VIDEO_OSD_TIMESTYLE          0
	#define MAX_VIDEO_OSD_TIMESTYPE          0

	#define MIN_VIDEO_OSD_TEXTFLAG           0
	#define MAX_VIDEO_OSD_TEXTFLAG           1
	
	#define MIN_VIDEO_OSD_FONTCOLOR          0
	#define MAX_VIDEO_OSD_FONTCOLOR          8
	#define MIN_VIDEO_OSD_FONTSTYLE          8
	#define MAX_VIDEO_OSD_FONTSTYLE          64
	#define MIN_VIDEO_OSD_FONTBLOD           0
	#define MAX_VIDEO_OSD_FONTBLOD           1
	#define MIN_VIDEO_OSD_FONTROTATE         0
	#define MAX_VIDEO_OSD_FONTROTATE         1
	#define MIN_VIDEO_OSD_FONTITALIC         0
	#define MAX_VIDEO_OSD_FONTITALIC         1
	#define MIN_VIDEO_OSD_FONTOUTLINE        0
	#define MAX_VIDEO_OSD_FONTOUTLINE        10
	#define MIN_VIDEO_OSD_DISPLAYX            0
	#define MAX_VIDEO_OSD_DISPLAYX            100
	#define MIN_VIDEO_OSD_DISPLAYY            0
	#define MAX_VIDEO_OSD_DISPLAYY            100
	#define MIN_VIDEO_OSD_DISPLAYH            0
	#define MAX_VIDEO_OSD_DISPLAYH            100
	#define MIN_VIDEO_OSD_DISPLAYW            0
	#define MAX_VIDEO_OSD_DISPLAYW            100

	//Image
	//base
	#define MIN_IMAGE_BASE_EXPMODE         0
	#define MAX_IMAGE_BASE_EXPMODE         2
	#define MIN_IMAGE_BASE_EXPVALUEMIN    125    //unit:us
	#define MAX_IMAGE_BASE_EXPVALUEMIN    40000
	#define MIN_IMAGE_BASE_EXPVALUEMAX    125    //unit:us
	#define MAX_IMAGE_BASE_EXPVALUEMAX    133333
	#define MIN_IMAGE_BASE_GAINMAX         0
	#define MAX_IMAGE_BASE_GAINMAX         60
	#define MIN_IMAGE_BASE_BRIGHT			  -255
	#define MAX_IMAGE_BASE_BRIGHT          255
	#define MIN_IMAGE_BASE_CONTRAST		  0
	#define MAX_IMAGE_BASE_CONTRAST        255
	#define MIN_IMAGE_BASE_SATURATION	  0
	#define MAX_IMAGE_BASE_SATURATION      255
	#define MIN_IMAGE_BASE_HUE			  -15
	#define MAX_IMAGE_BASE_HUE             15
	#define MIN_IMAGE_BASE_SHARP			  0
	#define MAX_IMAGE_BASE_SHARP           255

	//advance
	#define MIN_IMAGE_ADVANCE_METERMODE      0
	#define MAX_IMAGE_ADVANCE_METERMODE      3
	#define MIN_IMAGE_ADVANCE_BLCOMPFLAG     0
	#define MAX_IMAGE_ADVANCE_BLCOMPFLAG     1
	#define MIN_IMAGE_ADVANCE_DCIRISFLAG     0
	#define MAX_IMAGE_ADVANCE_DCIRISFLAG     1
	#define MIN_IMAGE_ADVANCE_LOCALEXP       0
	#define MAX_IMAGE_ADVANCE_LOCALEXP       256
	#define MIN_IMAGE_ADVANCE_MCTFSTRENGTH   0
	#define MAX_IMAGE_ADVANCE_MCTFSTRENGTH   512
	#define MIN_IMAGE_ADVANCE_DCIRISDUTY     100
	#define MAX_IMAGE_ADVANCE_DCIRISDUTY     999
	#define MIN_IMAGE_ADVANCE_AETARGETRATIO  25
	#define MAX_IMAGE_ADVANCE_AETARGETRATIO  400

	//DN
	#define MIN_IMAGE_DN_MODE        0
	#define MAX_IMAGE_DN_MODE        3
	#define MIN_IMAGE_DN_DURTIME     3    //unit: s
	#define MAX_IMAGE_DN_DURTIME     30
	#define MIN_IMAGE_DN_NTOD        0
	#define MAX_IMAGE_DN_NTOD        100
	#define MIN_IMAGE_DN_DTON        0
	#define MAX_IMAGE_DN_DTON        100
	#define MIN_IMAGE_DN_SCHEDFLAG  0
	#define MAX_IMAGE_DN_SCHEDFLAG  1
	#define MIN_IMAGE_DN_SCHEDTIME  0     //unit: s
	#define MAX_IMAGE_DN_SCHEDTIME  86400 //unit: s

	//WB
	#define MIN_IMAGE_WB_MODE       0
	#define MAX_IMAGE_WB_MODE       11
	#define MIN_IMAGE_WB_RGAIN      0
	#define MAX_IMAGE_WB_RGAIN      1023
	#define MIN_IMAGE_WB_BGAIN      0
	#define MAX_IMAGE_WB_BGAIN      1023

	//Audio
	#define MIN_AUDIO_CODEC        CODEC_G711A
	#define MAX_AUDIO_CODEC        CODEC_G726
	#define MIN_AUDIO_ID           0
	#define MAX_AUDIO_ID           1
	#define MIN_AUDIO_VOLUME       0
	#define MAX_AUDIO_VOLUME       100
	#define MIN_AUDIO_AEC_FALG     0
	#define MAX_AUDIO_AEC_FLAG     1
	#define MIN_AUDIO_FRAME		 1
	#define MAX_AUDIO_FRAME        50
	#define MIN_AUDIO_SAMPLEFREQ  8000
	#define MAX_AUDIO_SAMPLEFREQ  48000
    #define MIN_AUDIO_BITWIDTH    8
	#define MAX_AUDIO_BITWIDTH    24
	#define MIN_AUDIO_CHANNUM     1
	#define MAX_AUDIO_CHANNUM     2
	#define MIN_AUDIO_BITSRATE    16
	#define MAX_AUDIO_BITSRATE    320
	#define MIN_AUDIO_AEC_DELAY   -10000
	#define MAX_AUDIO_AEC_DELAY   10000

	//resolution ratio
	//#define RESOLUTION_VGA_WIDTH  
	
	//3D
    #define MIN_3D_POS_XY     0
	#define MAX_3D_POS_XY     8192
    #define MIN_3D_BOX_WH     0
	#define MAX_3D_BOX_WH     8192
	#define MIN_3D_PTZ_H      0
	#define MAX_3D_PTZ_H      35999
	#define MIN_3D_PTZ_V      0
	#define MAX_3D_PTZ_V      9000

	//PM
	#define MIN_PM_UNIT       0
	#define MAX_PM_UNIT       1
	#define MIN_PM_POS        0
	#define MAX_PM_POS        100
	#define MIN_PM_COLOR      0
	#define MAX_PM_COLOR      7
	#define MIN_PM_ACTION     0
	#define MAX_PM_ACTION     3

    typedef struct
    {
        uint16_t  s_Media;//MEDIA_VIDEO_H264 or MEDIA_VIDEO_MJPEG or MEDIA_VIDEO_MPEG4 or MEDIA_AUDIO_G711A or MEDIA_AUDIO_G711U
        uint16_t  s_Port;//default from 0
        MediaId   s_MediaId;//default from 0
        uint8_t  *s_StreamAddr;
        uint32_t  s_StreamSize;
        //uint32_t  s_FrameType;//VIDEO_I_FRAME or VIDEO_P_FRAME or VIDEO_B_FRAME or VIDEO_IDR_FRAME
        //uint32_t  s_FrameNum;
        struct timeval  s_PTS;
    } MediaEncInfo;

    typedef struct
    {
    	uint32_t  s_FrameType;//VIDEO_I_FRAME or VIDEO_P_FRAME or VIDEO_B_FRAME or VIDEO_IDR_FRAME
        uint32_t  s_FrameNum;
        size_t   s_Length;
        uint8_t  s_Reserved[1];
    } ExtMediaEncInfo;

    typedef struct
    {
        uint32_t  s_AudioId;//default from 0
        CodecType s_Codec;//CODEC_G711A orCODEC_G711U, default-G711A
        uint32_t  s_SampleFreq; //unit:hz, 8000hz, ...,default-8000hz
		uint32_t  s_BitWidth; //unit:bit, 8bit, ...,defualt-16bit
        uint16_t  s_ChannelNum;//default-1 sound channel		
		uint16_t  s_FrameRate; //default-50
		uint16_t  s_BitRate; //unit:kbps,default-64kbps
        uint16_t  s_Volume; //10%, 20%, 30% ...100%,default-100%
		uint16_t  s_AecFlag; //0-disable, 1-enable, Acoustic Echo Cancellation,default-0
		int16_t   s_AecDelayTime; //unit:ms, default-0
		uint8_t   s_Reserved[12];
    } AudioEncParam;

    typedef struct
    {
        uint32_t s_AudioId;//default from 0
        CodecType s_Codec;//CODEC_G711A orCODEC_G711U,default-G711A
        uint32_t  s_SampleFreq; //unit:hz, 8000hz, ...,default-8000hz
		uint32_t  s_BitWidth;   //unit:bit, 8bit, ...,default-16bit
		uint16_t  s_ChannelNum; //default-1 sound channel	
		uint16_t  s_FrameRate; //default-50
		uint16_t  s_BitRate; //unit:kbps,default-64kbps
        uint16_t  s_Volume; //10%, 20%, 30% ...100%,default-100%
		uint16_t  s_AecFlag; //0-disable, 1-enable, Acoustic Echo Cancellation,default-0
		int16_t   s_AecDelayTime; //unit:ms, default-0
		uint8_t   s_Reserved[12];
    } AudioDecParam;

	typedef struct
	{
		uint8_t s_Flag;     //0-disable, 1-enable, default-0
		uint8_t s_DateStyle; //date display style, 0-year-month-day,1-month-day-year etc. default-0
		uint8_t s_TimeStyle; //time display style, 0-hour:minute:sencond,  default-0
		uint8_t s_FontColor; //font color type, 0-black, 1-red, 2-blue,3-green,4-yellow,5-magenta,6-cyan,7-white,8-auto(self-adaption according to backgroud,range between white and balck), default-0
		uint8_t s_FontStyle;//Font size, range in16、 24、32、40, 48default-32
		uint8_t s_FontBlod; //0-no-blod, 1-blod, default-0
		uint8_t s_Rotate;   //0-no-rotate, 1-rotate, default-0
		uint8_t s_Italic;   //0-no-italic, 1-italic, default-0
		uint8_t s_Outline;  //font outline width, default-0
		uint8_t s_Reserved[3];
		uint16_t s_DisplayX; //default-0
		uint16_t s_DisplayY; //default-0
		uint16_t s_DisplayH; //default-50
		uint16_t s_DisplayW; //default-50
	}TimeDisplayData;	

	typedef struct
	{
		uint8_t s_Flag;     //0-disable, 1-enable, default-0
		uint8_t s_FontColor; //font color type, 0-black, 1-red, 2-blue,3-green,4-yellow,5-magenta,6-cyan,7-white,8-auto(self-adaption according to backgroud,range between white and balck), default-0
		uint8_t s_FontStyle;//Font size, range in16、 24、32、40,48, default-32
		uint8_t s_FontBlod; //0-no-blod, 1-blod, default-0
		uint8_t s_Rotate;   //0-no-rotate, 1-rotate, default-0
		uint8_t s_Italic;   //0-no-italic, 1-italic, default-0
		uint8_t s_Outline;  //font outline width, default-0
		uint8_t s_Reserved[3];
		uint16_t s_TextContentLen;
		int8_t s_TextContent[256];
		uint16_t s_DisplayX; // text area offset x, 0~100, 100 means 100% of encode width, default-0
		uint16_t s_DisplayY; // text area offset y, 0~100, 100 means 100% of encode height, default-0
		uint16_t s_DisplayH; // text area box height, 0~100, 100 means 100% of encode height, default-50
		uint16_t s_DisplayW; // text area box witdh, 0~100, 100 means 100% of encode width, default-50
	}TextDisplayData;	

	//osd parameter
	typedef struct
	{
		uint16_t s_StreamId;      //OSD Parameter id, range from 0-3
		uint16_t s_DisplayType;   //method of diaplay osd text, 0- opacity&no-blink, 1-hyaline&no-blink, 2-opacity&blink, 3-hyaline&blink
		uint32_t s_Flag;          //0-bmp disable, 1-bmp enable, default-0
		uint32_t s_Language;     //0/1 English, 2 Chinese, 3 Korean, default-0
		TimeDisplayData s_TimeDisplay;
		TextDisplayData s_TextDisplay[OSD_TEXT_NUM];		
	}VideoOSDParam;

	//encoding parameter
	typedef struct
	{
		uint16_t s_StreamId;    //source and stream Id, high-8bits is source id, low-8bits is stream id, default-0
		uint16_t s_EncodeType;  //0: none   1: H.264 (IAV_ENCODE_H264)   2: MJPEG  (IAV_ENCODE_MJPEG), default-1
		uint16_t s_FrameRate;   // frame rate,need change value in Q9 format, default-25
		uint16_t s_EncodeWidth; //encode width, default-1280
		uint16_t s_EncodeHeight; //encode height, default-720
		uint16_t s_BitRateType; //type of bitrate , 0:none, 1-CBR, 2- VBR, default-0
		uint32_t s_BitRateAverage; //average bitrate of cbr, default-4000
		uint32_t s_BitRateUp;       //maximum bitrate of vbr, default-4000
		uint32_t s_BitRateDown;     //minimum bitrate of vbr, default-1000
		uint16_t s_FrameInterval;//N of GOP, default-30
		uint16_t s_EncodeQulity; //quality of jpeg, range from 1~100, default-50
		uint8_t s_Rotate;         //range from 0~3, default-0,1-hor,2-ver,3-180degree
		uint8_t s_Reserved[3];
	}VideoEncodeParam;


	//image parameter
	typedef struct
	{
		int8_t s_ExposureMode;        //0-MW_ANTI_FLICKER_50HZ, 1-MW_ANTI_FLICKER_60HZ, 2-MW_ANTI_FLICKER_AUTO, default-0
		int8_t s_Reserved[3];
		int32_t s_ExposureValueMin;  //unit: microsecond, need change value in Q9 format, default-125
		int32_t s_ExposureValueMax;  //unit: microsecond, need change value in Q9 format, default-40000
		int16_t s_GainMax;            //unit:db, 0-ISO_AUTO/ISO_100,3-ISO_150,6-ISO_200,9-ISO_300,12-ISO_400, 15-ISO_600,
		                               //18-ISO_800, 24-ISO_1600, 30-ISO_3200, 36-ISO_6400, 42-ISO_12800, 48-ISO_25600,
		                               //54-ISO_51200,60-ISO_102400, default-36
		int16_t s_Brightness;        //valid range -255~255, default-0
		int16_t s_Contrast;          //valid range 0~255, default-128
		int16_t s_Saturation;        //valid range 0~255, default-64
		int16_t s_Hue;                //valid range -15~15, default-0
		int16_t s_Sharpness;         //valid range 0~255, default-128
	}ImageBaseParam;

	//image advanced parameter
	typedef struct
	{
	    int8_t s_MeteringMode;        //0-spot metering, 1-center metering, 2-average metering,3-custom metering, default-1
        int8_t s_BackLightCompFlag;   //0-disable, 1-enable, default-0
        int8_t s_DcIrisFlag;           //0-disable, 1-enable, default-0
        int8_t s_Reserved1[5];
        int16_t s_LocalExposure;       //WDR :0-disable, 1-Auto, unit in 64, 128 is 2x, valid range is 0~256, default-0
        int16_t s_MctfStrength;        //DigitalNoiseReduction:0-disable,unit in 32, 64 is 2x, valid range is 0~512, default-32
        int16_t s_DcIrisDuty;          //valid range is 100~999,, default-100
        int16_t s_AeTargetRatio;       //valid range is 25~400,unit in percentage, default-110
        int8_t  s_Reserver2[16];        
	}ImageAdvanceParam;

	//image AF parameter
	typedef struct
	{
	    int8_t s_AfFlag;            //0-disable, 1-enble, default-0
        int8_t s_AfMode;            //0-CAF, 1-SAF,3-MANUAL, 4-CALIB, 5-DEBUG
        int16_t s_LensType;         //0-LENS_CMOUNT_ID,10-LENS_CM8137_ID,  11-LENS_SY3510A_ID,2-LENS_IU072F_ID,  101-LENS_TAMRON18X_ID,102-LENS_JCD661_ID, 
                                     //103-LENS_SY6310_ID, 900-LENS_CUSTOM_ID
        int32_t s_ZmDist;
        
        int32_t s_FsDist;
        int32_t s_FsNear;
        int32_t s_FsFar;        
        int8_t  s_Reserver[4];        
	}ImageAfParam;

	//image DN parameter
	// 0: SUNDAY 1~6 MONDAY~SATDAY 7:all
	#define SCHEDULE_WEEK_DAYS  8
	typedef struct
	{
		uint32_t s_DnSchedFlag[SCHEDULE_WEEK_DAYS];  // 0-disable 1-enble, default-0
		uint32_t s_StartTime[SCHEDULE_WEEK_DAYS];    // 0~24*3600
		uint32_t s_EndTime[SCHEDULE_WEEK_DAYS];      // 0~24*3600 
	}DayNightSchedule;

	typedef struct
	{
	    int8_t s_DnMode;         //0-day mode,1-night mode, 2-auto ,3- timing mode, default-0                               
        int8_t s_DnDurtime;       //valid range is 3~30s, default-5
        int8_t s_NightToDayThr;   //valid range is 0~100,default-60
        int8_t s_DayToNightThr;   //valid range is 0~100,default-40
        DayNightSchedule s_DnSchedule;
        int8_t  s_Reserver[8];        
	}ImageDnParam;

	//image WB parameter
	typedef struct
	{
	    int16_t s_WbMode;        //0-MW_WB_AUTO,1-MW_WB_INCANDESCENT, 2-MW_WB_D4000,3-MW_WB_D5000,4-MW_WB_SUNNY, 5-MW_WB_CLOUDY,6-MW_WB_FLASH,
	                             //7-MW_WB_FLUORESCENT,8-MW_WB_FLUORESCENT_H, 9-MW_WB_UNDERWATER,10-MW_WB_CUSTOM, 11-MW_WB_CLOSE , 100-MW_WB_MODE_HOLD, default-0                                 
        int16_t s_WbRgain;       //valid range is 0~1023
        int16_t s_WbBgain;       //valid range is 0~1023
        int16_t  s_Reserver[5];        
	}ImageWbParam;

    //vin parameter
	typedef struct
	{
		uint32_t		s_VinFlag;          //0-disable, 1-enbale, default-1
		uint32_t		s_VinMode;          //0-AMBA_VIDEO_MODE_AUTO, 1-AMBA_VIDEO_MODE_320_240, 39-AMBA_VIDEO_MODE_720P25, 0xFFF0-AMBA_VIDEO_MODE_480I, ..., default-0
		uint32_t		s_VinFrameRate;    //range is 0-60,Q9 mode,  30-AMBA_VIDEO_FPS_30, ..., default-30
		uint32_t		s_VinMirrorPattern; //From 0 to 4 (4 is auto), default-0(nomal), 1-horizontal flip, 2-vertical flip, 3-horizontal&vertical flip
	    uint32_t		s_VinBayerPattern;  //From 0 to 4 (4 is auto), default-0
	}VideoInParam;

	//vout parameter
	typedef struct
	{
		uint32_t		s_VoutMode;      //0-AMBA_VIDEO_MODE_AUTO, 1-AMBA_VIDEO_MODE_320_240, 39-AMBA_VIDEO_MODE_720P25, 0xFFF0-AMBA_VIDEO_MODE_480I, default-0xFFF0
		uint32_t		s_VoutType;      //0-AMBA_VOUT_SINK_TYPE_AUTO, 1-AMBA_VOUT_SINK_TYPE_CVBS, 2-AMBA_VOUT_SINK_TYPE_SVIDEO, 3-AMBA_VOUT_SINK_TYPE_YPBPR
                                          //4-AMBA_VOUT_SINK_TYPE_HDMI, 5-AMBA_VOUT_SINK_TYPE_DIGITAL, 6-AMBA_VOUT_SINK_TYPE_MIPI, default-1
		uint32_t		s_VoutFlip;      //0-AMBA_VOUT_FLIP_NORMAL, 1-AMBA_VOUT_FLIP_HV, 2-AMBA_VOUT_FLIP_HORIZONTAL,3-AMBA_VOUT_FLIP_VERTICAL, default-0
	}VideoOutParam;

//3D
    typedef struct
	{
		E3dPosCmdIdx	s_CmdID;
		int32_t         s_StreamId;         //stream ID
		uint32_t	    s_X;				//the Xpos of top left corner
		uint32_t    	s_Y;				//the Ypos of top left corner
		int32_t			s_BoxWidth;			//the width of BOX
		int32_t			s_BoxHeight;		//the height of BOX
		uint32_t	    s_CurPtzX;          //the pos of hor of PTZ,range:0-35999
		uint32_t	    s_CurPtzY;          //the pos of ver of PTZ,range:0-9000
		uint32_t	    s_CurDptzX;         //the pos of hor of digital PTZ(no support)
		uint32_t	    s_CurDptzY;         //the pos of hor of digital PTZ(no support)
		uint32_t	    s_CurDptzZoom;		//the current DPTZ zoom, unit:0.1X(no support)
		int32_t	        s_ZoomRel;			//relative zoom(no support)
		int32_t			s_Reserved[6];
	} S3dPosParamIn;

    typedef  struct 
    {
        uint32_t	s_PanPos;			//the pos of hor of PTZ,range:0-35999
        uint32_t	s_TiltPos;			//the pos of ver of PTZ,range:0-9000
        uint32_t	s_ZoomPos;			//the pos of zoom
        uint32_t	s_DptzX;            //the pos of hor of digital PTZ(no support)
        uint32_t	s_DptzY;            //the pos of hor of digital PTZ(no support)
        uint32_t	s_DptzZoom;			//the current DPTZ zoom, unit:0.1X(no support)
        int32_t		s_Reserved[6];
    }S3dPosParamOut;

//PM
	typedef struct
	{
		int32_t		s_RegionUnit;       //0- percent ,1-pixel
		int32_t		s_Regionleft;       //s_RegionUnit==0:range:0-100;s_RegionUnit==1: XXpixel;
		int32_t		s_RegionTop;        //s_RegionUnit==0:range:0-100;s_RegionUnit==1: XXpixel;
		int32_t		s_RegionWidth;      //s_RegionUnit==0:range:0-100;s_RegionUnit==1: XXpixel;
		int32_t		s_RegionHeight;     //s_RegionUnit==0:range:0-100;s_RegionUnit==1: XXpixel;
		uint32_t	s_RegionColor;		// 0:Black, 1:Red, 2:Blue, 3:Green, 4:Yellow, 5:Magenta, 6:Cyan, 7:White
		uint32_t	s_PmAction;         //0-add ,1-remove,2-replace,3-remove all
	} PrivacyMaskParam;

//General Param--capability
	typedef struct
	{
		SysCpuIdType          s_CpuType;
		SysEncoderBoardIdType s_EncodeBoardType;
		SysVinIdType          s_SensorType;
		SysLensIdType         s_ZoomLensType;
		int32_t           s_Reserved;
	}GeneralParam_Capability;

	typedef struct 
	{
		uint32_t s_Min[2];
		uint32_t s_Max[2]; 
		uint32_t s_Avg[2]; 
	} IrutCalibrationParam;

	typedef struct
	{
		IrutCalibrationParam s_CalibrationParam[2];
		int32_t s_DayToNightThr;  //default -15
		int32_t s_NightToDayThr;  //default-30
		int32_t s_IrCutMode;      //0: ae 1:adc 2:ae_ex_ir_intensity 
		int32_t s_AdcMode;        //0: adc0 1:adc3 2:file 3:alarm in
		int32_t s_IrLightMode;   //0: 0x83 0=open 1=close 1: 0x83 0=close 1=open
	}GeneralParam_Ircut;

//


//media encode callback func type
    typedef void (*MediaEncCallBack)(void* UserDataPtr, MediaEncInfo*, ExtMediaEncInfo*);

//VIN&VOUT
   FD_HANDLE GMI_VinVoutCreate(SensorId  SId,  uint16_t  ChanId);
   void_t  GMI_VinVoutDestroy( FD_HANDLE VinVoutHd );
   GMI_RESULT GMI_VinVoutGetConfig( FD_HANDLE VinVoutHd, VideoInParam *VinParam, VideoOutParam *VoutParam);
   GMI_RESULT GMI_VinVoutSetConfig( FD_HANDLE VinVoutHd, VideoInParam *VinParam, VideoOutParam *VoutParam);


//video encode
    FD_HANDLE GMI_VideoEncCreate( uint16_t s_Port, MediaId s_MediaId, VideoEncodeParam* VidEncParamPtr );
    void_t  GMI_VideoEncDestroy( FD_HANDLE VideoEncHd );
    GMI_RESULT GMI_VideoEncStart( FD_HANDLE VideoEncHd );
    GMI_RESULT GMI_VideoEncStop( FD_HANDLE VideoEncHd );
    GMI_RESULT GMI_VideoEncGetConfig( FD_HANDLE VideoEncHd, VideoEncodeParam* VidEncParamPtr);
    GMI_RESULT GMI_VideoEncSetConfig( FD_HANDLE VideoEncHd, VideoEncodeParam* VidEncParamPtr);
	GMI_RESULT GMI_VideoOsdGetConfig( FD_HANDLE VideoEncHd, VideoOSDParam* VidOsdParamPtr);
    GMI_RESULT GMI_VideoOsdSetConfig( FD_HANDLE VideoEncHd, VideoOSDParam* VidOsdParamPtr);
    GMI_RESULT GMI_VideoEncSetCB( FD_HANDLE VideoEncHd, MediaEncCallBack VideoEncCB, void *UserDataPtr);
//force I frame
	GMI_RESULT GMI_ForceSetIdrFrame(FD_HANDLE VideoEncHd);

//image    ISP
    FD_HANDLE GMI_ImageOperateCreate(SensorId  SId,  uint16_t  ChanId);
    void_t  GMI_ImageOperateDestroy( FD_HANDLE ImageOptHd );
	
    GMI_RESULT GMI_ImageGetBaseConfig( FD_HANDLE ImageOptHd, ImageBaseParam* ImageBaseParamPtr);
    GMI_RESULT GMI_ImageSetBaseConfig( FD_HANDLE ImageOptHd, ImageBaseParam* ImageBaseParamPtr);
	
	GMI_RESULT GMI_ImageGetAdvanceConfig( FD_HANDLE ImageOptHd , ImageAdvanceParam* ImageAdvanceParamPtr);
	GMI_RESULT GMI_ImageSetAdvanceConfig( FD_HANDLE ImageOptHd , ImageAdvanceParam* ImageAdvanceParamPtr);

   	GMI_RESULT GMI_ImageGetAfConfig( FD_HANDLE ImageOptHd , ImageAfParam* ImageAfParamPtr);
    GMI_RESULT GMI_ImageSetAfConfig( FD_HANDLE ImageOptHd , ImageAfParam* ImageAfParamPtr);


   	GMI_RESULT GMI_ImageGetDnConfig( FD_HANDLE ImageOptHd , ImageDnParam* ImageDnParamPtr);
	GMI_RESULT GMI_ImageSetDnConfig( FD_HANDLE ImageOptHd , ImageDnParam* ImageDnParamPtr);
	
   	GMI_RESULT GMI_ImageGetWbConfig( FD_HANDLE ImageOptHd , ImageWbParam* ImageWbParamPtr);
    GMI_RESULT GMI_ImageSetWbConfig( FD_HANDLE ImageOptHd , ImageWbParam* ImageWbParamPtr);

//Auto Focus
    FD_HANDLE  GMI_AutoFocusCreate(SensorId  SId); 
    void_t GMI_AutoFocusDestroy(FD_HANDLE  AutoFocusHd);
	GMI_RESULT  GMI_AutoFocusStart(FD_HANDLE  AutoFocusHd);
    GMI_RESULT  GMI_AutoFocusStop(FD_HANDLE  AutoFocusHd);
	GMI_RESULT	GMI_AutoFocusPause(FD_HANDLE  AutoFocusHd, int8_t ControlStatus);
	GMI_RESULT  GMI_AutoFocusGlobalScan(FD_HANDLE AutoFocusHd);
	GMI_RESULT  GMI_AutoFocusSetMode(FD_HANDLE  AutoFocusHd, int32_t AFMode);
	GMI_RESULT GMI_AFEventNotify(FD_HANDLE	AutoFocusHd, int32_t EventType,  uint8_t *ExtData, uint32_t Length);
	GMI_RESULT	GMI_FocusPositionGet (FD_HANDLE  AutoFocusHd, int32_t *CurFocusPos);
	GMI_RESULT GMI_FocusPositionSet (FD_HANDLE  AutoFocusHd, int32_t  FocusPos);
	GMI_RESULT  GMI_FocusRangeGet (FD_HANDLE  AutoFocusHd, int32_t *MinFocusPos, int32_t *MaxFocusPos);
	GMI_RESULT  GMI_FocusMotorReset(FD_HANDLE  AutoFocusHd);
	GMI_RESULT  GMI_AfCtrol (FD_HANDLE  AutoFocusHd, int8_t  AfDirMode);
	GMI_RESULT	GMI_AfStepSet (FD_HANDLE	AutoFocusHd, int32_t  AfStep);

//Zoom
    FD_HANDLE  GMI_ZoomCreate(SensorId  SId);
    void_t GMI_ZoomDestroy(FD_HANDLE  ZoomHd);
	GMI_RESULT	GMI_ZoomPositionGet (FD_HANDLE	ZoomHd, int32_t *CurZoomPos);
	GMI_RESULT  GMI_ZoomPositionSet (FD_HANDLE  ZoomHd, int32_t  ZoomPos);
	GMI_RESULT	GMI_ZoomRangeGet (FD_HANDLE  ZoomHd, int32_t *MinZoomPos, int32_t *MaxZoomPos);
	GMI_RESULT  GMI_ZoomMotorReset(FD_HANDLE  ZoomHd);
	GMI_RESULT  GMI_ZoomCtrol (FD_HANDLE  ZoomHd, int8_t  ZoomMode);
	GMI_RESULT	GMI_ZoomStepSet (FD_HANDLE	ZoomHd, int32_t  ZoomStep);


//audio encode
    FD_HANDLE  GMI_AudioEncCreate( uint16_t AudioPort, AudioEncParam *AudEncParamPtr);
    void_t GMI_AudioEncDestroy( FD_HANDLE AudioEncHd );
    GMI_RESULT GMI_AudioEncStart( FD_HANDLE AudioEncHd );
    GMI_RESULT GMI_AudioEncStop( FD_HANDLE AudioEncHd);
    GMI_RESULT GMI_AudioEncSetCB( FD_HANDLE AudioEncHd, MediaEncCallBack AudioEncCB, void *UserDataPtr);
	GMI_RESULT GMI_AudioSetEncConfig(FD_HANDLE AudioEncHd, AudioEncParam *AudEncParamPtr);
	GMI_RESULT GMI_AudioGetEncConfig(FD_HANDLE AudioEncHd, AudioEncParam *AudEncParamPtr);

//audio decode
    FD_HANDLE  GMI_AudioDecCreate( uint16_t AudioPort, AudioDecParam* AudDecParamPtr );
    void_t GMI_AudioDecDestroy( FD_HANDLE AudioDecHd );
    GMI_RESULT GMI_AudioDecOneFrame( FD_HANDLE AudioDecHd, uint8_t *AudStreamPtr, uint32_t AudStreamLen);
	GMI_RESULT GMI_AudioSetDecConfig(FD_HANDLE AudioDecHd, AudioDecParam *AudDecParamPtr);
	GMI_RESULT GMI_AudioGetDecConfig(FD_HANDLE AudioDecHd, AudioDecParam *AudDecParamPtr);

//3D
    GMI_RESULT GMI_3DPosProcess(S3dPosParamIn *ParamIn, S3dPosParamOut *ParamOut, void *ExtDataPtr, int32_t *ExtDataType);

//PM
    GMI_RESULT GMI_PrivacyMaskSetConfig( FD_HANDLE VideoEncHd, PrivacyMaskParam* PrivacyMaskParamPtr);

//General Param
	GMI_RESULT GMI_GeneralParamSetConfig(General_Param ParamType, void_t *GeneralParamPtr);
    GMI_RESULT GMI_GeneralParamGetConfig(General_Param *ParamType, void_t *GeneralParamPtr);

#ifdef __cplusplus
}
#endif

#endif



