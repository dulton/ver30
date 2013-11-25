
#include <stream_control.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <memory>
#include <config/gmi_config_api.h>
#include <rudp/gmi_rudp_api.h>
#include <ipc_fw_v3.x_resource.h>


int __GetSdkPort()
{
    GMI_RESULT gmiret;
    FD_HANDLE xmlhd=NULL;
    int serverport;

    gmiret = GMI_XmlOpen(GMI_RESOURCE_XML,&xmlhd);
    if (gmiret != GMI_SUCCESS)
    {
        goto set_default;
    }
    gmiret = GMI_XmlRead(xmlhd,GMI_SYS_SDK_PORT_PATH,
                         GMI_SDK_TO_SYS_SERVER_PORT_ITEM,
                         SDK_TO_SYS_SERVER_PORT,
                         &serverport,GMI_CONFIG_READ_ONLY);
    if (gmiret != GMI_SUCCESS)
    {
        goto set_default;
    }
    GMI_XmlFileSave(xmlhd);
    xmlhd = NULL;

    return serverport;


set_default:
    if (xmlhd)
    {
        GMI_XmlFileSave(xmlhd);
    }
    xmlhd= NULL;
    return SDK_TO_SYS_SERVER_PORT;
}

int __GetBindPort()
{
    GMI_RESULT gmiret;
    FD_HANDLE xmlhd=NULL;
    int serverport;

    gmiret = GMI_XmlOpen(GMI_RESOURCE_XML,&xmlhd);
    if (gmiret != GMI_SUCCESS)
    {
        goto set_default;
    }
    gmiret = GMI_XmlRead(xmlhd,GMI_SYS_SDK_PORT_PATH,
                         GMI_SYS_SERVER_TO_SDK_PORT_ITEM ,
                         SYS_SERVER_TO_SDK_PORT,
                         &serverport,GMI_CONFIG_READ_ONLY);
    if (gmiret != GMI_SUCCESS)
    {
        goto set_default;
    }
    GMI_XmlFileSave(xmlhd);
    xmlhd = NULL;

    return serverport;


set_default:
    if (xmlhd)
    {
        GMI_XmlFileSave(xmlhd);
    }
    xmlhd= NULL;
    return SYS_SERVER_TO_SDK_PORT;
}



#define  STREAM_ENCODE_PATH "/Config/Video/Encode/"
#define  STREAM_NUM_ITEM  "StreamNum"


#define  STREAM_ENCODETYPE_ITEM       "EncodeType"
#define  STREAM_STREAMTYPE_ITEM       "StreamType"
#define  STREAM_ENCODEWITH_ITEM       "EncodeWidth"
#define  STREAM_ENCODEHEIGHT_ITEM     "EncodeHeight"
#define  STREAM_BITRATETYPE_ITEM      "BitRateType"
#define  STREAM_BITRATEAVERAGE_ITEM   "BitRateAverage"
#define  STREAM_BITRATEUP_ITEM        "BitRateUp"
#define  STREAM_BITRATEDOWN_ITEM      "BitRateDown"
#define  STREAM_FRAMERATE_ITEM        "FrameRate"
#define  STREAM_FRAMEINTERVAL_ITEM    "FrameInterval"
#define  STREAM_ENCODEQUALITY_ITEM    "EncodeQuality"
#define  STREAM_ROTATE_ITEM           "Rotate"

#define  STREAM_AUDIO_PATH "/Config/Audio/Encode/"

#define  STREAM_AUDIO_ENCODE_ITEM          "EncodeType"
#define  STREAM_AUDIO_CHANNEL_ITEM         "Channel"
#define  STREAM_AUDIO_BITSPERSAMPLE_ITEM   "BitsPerSample"
#define  STREAM_AUDIO_SAMPLEPERSEC_ITEM    "SamplePerSec"

#define ERROR_INFO(...) do{fprintf(stderr,"%s:%d[%ld]\t",__FILE__,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)
#define DEBUG_INFO(...) do{fprintf(stderr,"%s:%d[%ld]\t",__FILE__,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);}while(0)


void DebugSysPkgEncodeCfg(SysPkgEncodeCfg* pCfg)
{
	fprintf(stdout,"videoid %d\n",pCfg->s_VideoId);
	fprintf(stdout,"streamtype %d\n",pCfg->s_StreamType);
	fprintf(stdout,"compression %d\n",pCfg->s_Compression);
	fprintf(stdout,"picwidth %d\n",pCfg->s_PicWidth);
	fprintf(stdout,"picheight %d\n",pCfg->s_PicHeight);
	fprintf(stdout,"bitratectrl %d\n",pCfg->s_BitrateCtrl);
	fprintf(stdout,"quality %d\n",pCfg->s_Quality);
	fprintf(stdout,"fps %d\n",pCfg->s_FPS);
	fprintf(stdout,"bitrateaverage %d\n",pCfg->s_BitRateAverage);
	fprintf(stdout,"bitrateup %d\n",pCfg->s_BitRateUp);
	fprintf(stdout,"bitratedown %d\n",pCfg->s_BitRateDown);
	fprintf(stdout,"gop %d\n",pCfg->s_Gop);
	fprintf(stdout,"rotate %d\n",pCfg->s_Rotate);
	fprintf(stdout,"flag %d\n",pCfg->s_Flag);
	return;
}

void DebugSysPkgAudioCfg(SysPkgAudioEncodeCfg *pAudio)
{
	fprintf(stdout,"EncodeType %d\n",pAudio->s_EncodeType);
	fprintf(stdout,"Channel %d\n",pAudio->s_Chan);
	fprintf(stdout,"BitPerSample %d\n",pAudio->s_BitsPerSample);
	fprintf(stdout,"SamplePerSec %d\n",pAudio->s_SamplesPerSec);
	fprintf(stdout,"\n");
	return;
}

int GetStreamInfo(const char* conffile,std::vector<SysPkgEncodeCfg*>& cfgs)
{
    SysPkgEncodeCfg* pNewCfg=NULL;
    GMI_RESULT gmiret;
    FD_HANDLE xmlhd=NULL;
    int ret,count =0;
    unsigned int i;
    int maxpath=1024;
    std::auto_ptr<char> pFmtPath2(new char[maxpath]);
    char *pFmtPath = pFmtPath2.get();
    int streamlen =32;
    std::auto_ptr<char> pFmtStream2(new char[streamlen]);
    char *pFmtStream = pFmtStream2.get();
    int value;

    if (cfgs.size() > 0)
    {
		ERROR_INFO("\n");
        return -EINVAL;
    }

    gmiret = GMI_XmlOpen(conffile,&xmlhd);
    if (gmiret != GMI_SUCCESS)
    {
        ret =-EIO;
		ERROR_INFO("\n");
        goto fail;
    }

    gmiret = GMI_XmlRead(xmlhd,STREAM_ENCODE_PATH,STREAM_NUM_ITEM,0,&count,GMI_CONFIG_READ_ONLY);
    if (gmiret != GMI_SUCCESS)
    {
        ret = -errno ? -errno : -1;
		ERROR_INFO("\n");
        goto fail;
    }

	DEBUG_INFO("[%s].[%s]count %d\n",STREAM_ENCODE_PATH,STREAM_NUM_ITEM,count);
    for (i= 0 ; i < (unsigned int)count ; i ++)
    {
        ret = snprintf(pFmtStream,streamlen,"Stream%d",i);
        if (ret < 0)
        {
            ret = -errno ? -errno : -1;
        	ERROR_INFO("\n");
            goto fail;
        }
        if (ret >= streamlen)
        {
            ret = -ENOSPC;
        	ERROR_INFO("\n");
            goto fail;
        }

        ret = snprintf(pFmtPath,maxpath,"%s%s/",STREAM_ENCODE_PATH,pFmtStream);
        if (ret < 0)
        {
            ret = -errno ? -errno : -1;
        	ERROR_INFO("\n");
            goto fail;
        }
        if (ret >= maxpath)
        {
            ret = -ENOSPC;
        	ERROR_INFO("\n");
            goto fail;
        }

        assert(pNewCfg == NULL);
        pNewCfg = (typeof(pNewCfg))calloc(sizeof(*pNewCfg),1);
        if (pNewCfg == NULL)
        {
            ret = -ENOMEM;
        	ERROR_INFO("\n");
            goto fail;
        }
		pNewCfg->s_VideoId = 1;

        gmiret = GMI_XmlRead(xmlhd,pFmtPath,STREAM_ENCODETYPE_ITEM,0,&value,GMI_CONFIG_READ_ONLY);
        if (gmiret != GMI_SUCCESS)
        {
            ret = -errno ? -errno : -1;
        	ERROR_INFO("\n");
            goto fail;
        }

        switch(value)
        {
        case 1:
            pNewCfg->s_Compression = SYS_COMP_H264;
            break;
        case 2:
            pNewCfg->s_Compression = SYS_COMP_MJPEG;
            break;
        default:
            ret = -EINVAL;
        	ERROR_INFO("%s/%svalue %d\n",pFmtPath,STREAM_ENCODETYPE_ITEM,value);
            goto fail;
        }
		gmiret = GMI_XmlRead(xmlhd,pFmtPath,STREAM_STREAMTYPE_ITEM,1,&value,GMI_CONFIG_READ_ONLY);
		if (gmiret != GMI_SUCCESS)
		{
			ret = -errno ?-errno : -1;
			ERROR_INFO("\n");
			goto fail;
		}
		pNewCfg->s_StreamType = value;

        gmiret = GMI_XmlRead(xmlhd,pFmtPath,STREAM_ENCODEWITH_ITEM,0,&value,GMI_CONFIG_READ_ONLY);
        if (gmiret != GMI_SUCCESS)
        {
            ret = -errno ? -errno : -1;
        	ERROR_INFO("\n");
            goto fail;
        }

        pNewCfg->s_PicWidth = value;


        gmiret = GMI_XmlRead(xmlhd,pFmtPath,STREAM_ENCODEHEIGHT_ITEM,0,&value,GMI_CONFIG_READ_ONLY);
        if (gmiret != GMI_SUCCESS)
        {
            ret = -errno ? -errno : -1;
        	ERROR_INFO("\n");
            goto fail;
        }
        pNewCfg->s_PicHeight = value;

        gmiret = GMI_XmlRead(xmlhd,pFmtPath,STREAM_BITRATETYPE_ITEM,0,&value,GMI_CONFIG_READ_ONLY);
        if (gmiret != GMI_SUCCESS)
        {
            ret = -errno ? -errno : -1;
        	ERROR_INFO("\n");
            goto fail;
        }
        pNewCfg->s_BitrateCtrl = value;

        gmiret = GMI_XmlRead(xmlhd,pFmtPath,STREAM_BITRATEAVERAGE_ITEM,0,&value,GMI_CONFIG_READ_ONLY);
        if (gmiret != GMI_SUCCESS)
        {
            ret = -errno ? -errno : -1;
        	ERROR_INFO("\n");
            goto fail;
        }
        pNewCfg->s_BitRateAverage = value;

        gmiret = GMI_XmlRead(xmlhd,pFmtPath,STREAM_BITRATEUP_ITEM,0,&value,GMI_CONFIG_READ_ONLY);
        if (gmiret != GMI_SUCCESS)
        {
            ret = -errno ? -errno : -1;
        	ERROR_INFO("\n");
            goto fail;
        }
        pNewCfg->s_BitRateUp= value;

        gmiret = GMI_XmlRead(xmlhd,pFmtPath,STREAM_BITRATEDOWN_ITEM,0,&value,GMI_CONFIG_READ_ONLY);
        if (gmiret != GMI_SUCCESS)
        {
            ret = -errno ? -errno : -1;
        	ERROR_INFO("\n");
            goto fail;
        }
        pNewCfg->s_BitRateDown= value;


        gmiret = GMI_XmlRead(xmlhd,pFmtPath,STREAM_FRAMERATE_ITEM,0,&value,GMI_CONFIG_READ_ONLY);
        if (gmiret != GMI_SUCCESS)
        {
            ret = -errno ? -errno : -1;
        	ERROR_INFO("\n");
            goto fail;
        }
        pNewCfg->s_FPS = value;


        gmiret = GMI_XmlRead(xmlhd,pFmtPath,STREAM_FRAMEINTERVAL_ITEM,0,&value,GMI_CONFIG_READ_ONLY);
        if (gmiret != GMI_SUCCESS)
        {
            ret = -errno ? -errno : -1;
        	ERROR_INFO("\n");
            goto fail;
        }
        pNewCfg->s_Gop= value;

        gmiret = GMI_XmlRead(xmlhd,pFmtPath,STREAM_ENCODEQUALITY_ITEM,0,&value,GMI_CONFIG_READ_ONLY);
        if (gmiret != GMI_SUCCESS)
        {
            ret = -errno ? -errno : -1;
        	ERROR_INFO("\n");
            goto fail;
        }
        pNewCfg->s_Quality = value;

        gmiret = GMI_XmlRead(xmlhd,pFmtPath,STREAM_ROTATE_ITEM,0,&value,GMI_CONFIG_READ_ONLY);
        if (gmiret != GMI_SUCCESS)
        {
            ret = -errno ? -errno : -1;
        	ERROR_INFO("\n");
            goto fail;
        }
        pNewCfg->s_Rotate= value;

		pNewCfg->s_Flag = i;

		DebugSysPkgEncodeCfg(pNewCfg);

        cfgs.push_back(pNewCfg);
        pNewCfg = NULL;

    }

    assert(pNewCfg == NULL);
    assert((int)cfgs.size() == count);

    GMI_XmlFileSave(xmlhd);
    xmlhd = NULL;


    return count;

fail:
    if (pNewCfg)
    {
        free(pNewCfg);
    }
    pNewCfg = NULL;
    while(cfgs.size() > 0)
    {
        pNewCfg = cfgs[0];
        cfgs.erase(cfgs.begin());
        free(pNewCfg);
        pNewCfg = NULL;
    }
    if (xmlhd)
    {
        GMI_XmlFileSave(xmlhd);
    }
    xmlhd = NULL;
    return ret;
}

int GetAudioInfo(const char* pConfigFile,std::vector<SysPkgAudioEncodeCfg*>& pAudioCfg)
{
	SysPkgAudioEncodeCfg* pNewCfg=NULL;
    GMI_RESULT gmiret;
    FD_HANDLE xmlhd=NULL;
	int ret;
    int maxpath=1024;
    std::auto_ptr<char> pFmtPath2(new char[maxpath]);
    char *pFmtPath = pFmtPath2.get();
	int val;
	

	gmiret = GMI_XmlOpen(pConfigFile,&xmlhd);
	if (gmiret != GMI_SUCCESS)
	{
		ret = -errno ? -errno : -1;
		goto fail;
	}

	snprintf(pFmtPath,maxpath,"%s",STREAM_AUDIO_PATH);
	gmiret = GMI_XmlRead(xmlhd,pFmtPath,STREAM_AUDIO_ENCODE_ITEM,1,&val,GMI_CONFIG_READ_ONLY);
	if (gmiret != GMI_SUCCESS)
	{
		ret = -errno ? -errno : -1;
		ERROR_INFO("\n");
		goto fail;
	}

	pNewCfg =(SysPkgAudioEncodeCfg*) calloc(sizeof(*pNewCfg),1);
	if (pNewCfg == NULL)
	{
		ret = -errno ? -errno : -1;
		ERROR_INFO("\n");
		goto fail;
	}
	pNewCfg->s_AudioId = 1;
	if (val == 4)
	{
		val = 1;
	}
	pNewCfg->s_EncodeType = val;

	gmiret = GMI_XmlRead(xmlhd,pFmtPath,STREAM_AUDIO_CHANNEL_ITEM,1,&val,GMI_CONFIG_READ_ONLY);
	if (gmiret != GMI_SUCCESS)
	{
		ret = -errno ? -errno : -1;
		ERROR_INFO("\n");
		goto fail;
	}
	pNewCfg->s_Chan = val;

	gmiret = GMI_XmlRead(xmlhd,pFmtPath,STREAM_AUDIO_BITSPERSAMPLE_ITEM,16,&val,GMI_CONFIG_READ_ONLY);
	if (gmiret != GMI_SUCCESS)
	{
		ret = -errno ? -errno : -1;
		ERROR_INFO("\n");
		goto fail;
	}

	pNewCfg->s_BitsPerSample = val;

	gmiret = GMI_XmlRead(xmlhd,pFmtPath,STREAM_AUDIO_SAMPLEPERSEC_ITEM,8000,&val,GMI_CONFIG_READ_ONLY);
	if (gmiret != GMI_SUCCESS)
	{
		ret = -errno ? -errno : -1;
		ERROR_INFO("\n");
		goto fail;
	}


	pNewCfg->s_SamplesPerSec = val;

	DebugSysPkgAudioCfg(pNewCfg);
	pAudioCfg.push_back(pNewCfg);

	

	GMI_XmlFileSave(xmlhd);
	return 0;
fail:
	if (xmlhd)
	{
		GMI_XmlFileSave(xmlhd);
	}
	xmlhd = NULL;
	if (pNewCfg)
	{
		free(pNewCfg);
	}
	pNewCfg = NULL;
	return ret;
}

void Usage(int exitcode,const char* fmt,...)
{
    FILE* fp=stderr;
    va_list ap;

    if (exitcode == 0)
    {
        fp = stdout;
    }

    if (fmt)
    {
        va_start(ap,fmt);
        vfprintf(fp,fmt,ap);
        fprintf(fp,"\n");
    }

    fprintf(fp,"forgestream [OPTIONS] optconfigfile localport remoteport\n");
    fprintf(fp,"\t--start start stream\n");
    fprintf(fp,"\t--stop  stop stream\n");
    fprintf(fp,"\t--pause pause stream\n");
    fprintf(fp,"\t--resume resume stream\n");
    fprintf(fp,"\t--query query stream state\n");

    exit(exitcode);
}

int main(int argc,char* argv[])
{
    int ret;
    GMI_RESULT gmiret;
    std::vector<SysPkgEncodeCfg*> cfgs;
	std::vector<SysPkgAudioEncodeCfg*> audiocfgs;
    std::auto_ptr<SysPkgEncodeCfg> pCfgs2(new SysPkgEncodeCfg);
	std::auto_ptr<SysPkgAudioEncodeCfg> pAudio2(new SysPkgAudioEncodeCfg);
	SysPkgAudioEncodeCfg *pAudios=pAudio2.get();
    SysPkgEncodeCfg* pCfgs=pCfgs2.get();
    unsigned int i;
	FD_HANDLE hd=NULL;
	int sdkport=-1;
	int localport=-1;
    if (argc < 2)
    {
        Usage(3,NULL);
    }

	if (argc >= 4)
	{
		localport = atoi(argv[3]);
	}
	else
	{
		localport = __GetBindPort();
	}

	if (argc >= 5)
	{
		sdkport = atoi(argv[4]);
	}
	else
	{
		sdkport = __GetSdkPort();
	}

	hd = GMI_RudpSocket(localport);
	if (hd ==NULL)
	{
		ret = -errno ? -errno : -1;
		ERROR_INFO("could not bind port %d\n",localport);
		goto out;
	}
	DEBUG_INFO("localport %d sdkport %d\n",localport ,sdkport);

    if (strcmp(argv[1],"--start") == 0)
    {
        if (argc < 3)
        {
            Usage(3,"%s need config file",argv[1]);
        }

        ret = GetStreamInfo(argv[2],cfgs);
        if (ret < 0)
        {
        	ERROR_INFO("\n");
            goto out;
        }

		ret = GetAudioInfo(argv[2],audiocfgs);
		if (ret < 0)
		{
			ERROR_INFO("\n");
			goto out;
		}

        pCfgs2.reset(new SysPkgEncodeCfg[cfgs.size()]);
        pCfgs = pCfgs2.get();
		pAudio2.reset(new SysPkgAudioEncodeCfg[audiocfgs.size()]);
		pAudios = pAudio2.get();

        for (i=0; i<cfgs.size(); i++)
        {
            memcpy(&(pCfgs[i]),cfgs[i],sizeof(*pCfgs));
        }

		for (i=0;i<audiocfgs.size();i++)
		{
			memcpy(&(pAudios[i]),audiocfgs[i],sizeof(*pAudios));
		}

        gmiret = StartStreamTransfer(hd,sdkport,pCfgs,cfgs.size(),pAudios,audiocfgs.size(),10);
        if (gmiret != GMI_SUCCESS)
        {
            ret = -errno ? -errno : -1;
        	ERROR_INFO("\n");
            goto out;
        }

        fprintf(stdout,"start succ\n");

    }
    else if (strcmp(argv[1],"--stop") == 0)
    {
        gmiret = StopStreamTransfer(hd,sdkport,10);
        if (gmiret != GMI_SUCCESS)
        {
            ret = -errno ? -errno : -1;
            goto out;
        }
        fprintf(stdout,"stop succ\n");
    }
    else if (strcmp(argv[1],"--resume") == 0)
    {
        if (argc < 3)
        {
            Usage(3,"%s need config file",argv[1]);
        }

        ret = GetStreamInfo(argv[2],cfgs);
        if (ret < 0)
        {
            goto out;
        }

		ret = GetAudioInfo(argv[2],audiocfgs);
		if (ret < 0)
		{
			ERROR_INFO("\n");
			goto out;
		}

        pCfgs2.reset(new SysPkgEncodeCfg[cfgs.size()]);
        pCfgs = pCfgs2.get();
		pAudio2.reset(new SysPkgAudioEncodeCfg[audiocfgs.size()]);
		pAudios = pAudio2.get();

        for (i=0; i<cfgs.size(); i++)
        {
            memcpy(&(pCfgs[i]),cfgs[i],sizeof(*pCfgs));
        }

		for (i=0;i<audiocfgs.size();i++)
		{
			memcpy(&(pAudios[i]),audiocfgs[i],sizeof(*pAudios));
		}

        gmiret = ResumeStreamTransfer(hd,sdkport,pCfgs,cfgs.size(),pAudios,audiocfgs.size(),10);
        if (gmiret != GMI_SUCCESS)
        {
            ret = -errno ? -errno : -1;
            goto out;
        }
        fprintf(stdout,"resume succ\n");
    }
    else if (strcmp(argv[1],"--pause") == 0)
    {
        gmiret = PauseStreamTransfer(hd,sdkport,10);
        if (gmiret != GMI_SUCCESS)
        {
            ret = -errno ? -errno : -1;
            goto out;
        }
        fprintf(stdout,"pause succ\n");
    }
    else if (strcmp(argv[1],"--query")==0)
    {
        int started;
        gmiret = QueryStreamTransfer(hd,sdkport,10,&started);
        if (gmiret != GMI_SUCCESS)
        {
            ret = -errno ? -errno : -1;
            goto out;
        }
        fprintf(stdout,"stream state %d\n",started);
    }
    else
    {
        Usage(3,NULL);
    }

out:
	if (hd != NULL)
	{
		GMI_RudpSocketClose(hd);
	}
	hd = NULL;
    return ret;
}


