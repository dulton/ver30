/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2010 Live Networks, Inc.  All rights reserved.
// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, from a MPEG-4 video file.
// Implementation

#include "H264VideoFileServerMediaSubsession.hh"

#include "ByteStreamFileSource.hh"
#include <fcntl.h>
#include "MyH264VideoRTPSink.hh"
#include "MyH264VideoStreamFramer.hh"
#if 0 // do not need JPEG function for now
#include "MyJPEGVideoRTPSink.hh"
#include "MyJPEGVideoSource.hh"
#endif
#include <stdio.h>
#if defined( __linux__ )
#include <sys/ioctl.h>
#endif

H264VideoFileServerMediaSubsession*
H264VideoFileServerMediaSubsession::createNew(UsageEnvironment& env,
        char const* fileName,
        Boolean reuseFirstSource,
        uint16_t IPCMediaDataDispatchServerPort,
        uint16_t IPCMediaDataDispatchClientPort,
        portNumBits initialPortNum,
        netAddressBits multicastDest)
{
    return new H264VideoFileServerMediaSubsession(env, fileName, reuseFirstSource, IPCMediaDataDispatchServerPort, IPCMediaDataDispatchClientPort, initialPortNum, multicastDest);
}

H264VideoFileServerMediaSubsession
::H264VideoFileServerMediaSubsession(UsageEnvironment& env,
                                     char const* fileName, Boolean reuseFirstSource, uint16_t IPCMediaDataDispatchServerPort, uint16_t IPCMediaDataDispatchClientPort,
                                     portNumBits initialPortNum,
                                     netAddressBits multicastDest)
    : FileServerMediaSubsession(env, fileName, reuseFirstSource, initialPortNum, multicastDest),
      fDoneFlag(0),
      fEncType(0),
      fIPCMediaDataDispatchServerPort( IPCMediaDataDispatchServerPort ), fIPCMediaDataDispatchClientPort( IPCMediaDataDispatchClientPort )
{
//    setServerAddressAndPortForSDP(0, 50000);
}

H264VideoFileServerMediaSubsession
::~H264VideoFileServerMediaSubsession()
{
}

static void afterPlayingDummy(void* clientData)
{
    H264VideoFileServerMediaSubsession* subsess
    = (H264VideoFileServerMediaSubsession*)clientData;
    subsess->afterPlayingDummy1();
}

void H264VideoFileServerMediaSubsession::afterPlayingDummy1()
{
    // Unschedule any pending 'checking' task:
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    // Signal the event loop that we're done:
    setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData)
{
    H264VideoFileServerMediaSubsession* subsess
    = (H264VideoFileServerMediaSubsession*)clientData;
    subsess->checkForAuxSDPLine1();
}

void H264VideoFileServerMediaSubsession::checkForAuxSDPLine1()
{
    // If encode stream is mjpeg, the done flag is also needed to be set.
    // If not set for mjpeg, rtsp_server will not come out of  DESBRIBE command processing from client
    // Modified by Zhaoyang
    //if (fDummyRTPSink->auxSDPLine() != NULL) {
#if defined( USE_V3_3_CODE )
    if (fEncType == IAV_ENCODE_MJPEG || fDummyRTPSink->auxSDPLine() != NULL)
    {
#else
    if (fDummyRTPSink->auxSDPLine() != NULL)
    {
#endif
        // Signal the event loop that we're done:
        setDoneFlag();
    }
    else
    {
        // try again after a brief delay:
        int uSecsToDelay = 100000; // 100 ms
        nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
                     (TaskFunc*)checkForAuxSDPLine, this);
    }
}

char const* H264VideoFileServerMediaSubsession
::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource)
{
    // Note: For MPEG-4 video files, the 'config' information isn't known
    // until we start reading the file.  This means that "rtpSink"s
    // "auxSDPLine()" will be NULL initially, and we need to start reading
    // data from our file until this changes.
    fDummyRTPSink = rtpSink;

    // Start reading the file:
    fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);

    // Check whether the sink's 'auxSDPLine()' is ready:
    checkForAuxSDPLine(this);

    envir().taskScheduler().doEventLoop(&fDoneFlag);

    char const* auxSDPLine = fDummyRTPSink->auxSDPLine();
    return auxSDPLine;
}

FramedSource* H264VideoFileServerMediaSubsession
::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate)
{
    estBitrate = 500; // 500 kbps, estimate
    int streamId = -1;
    if (strncmp(fFileName, "live_stream1", 12) == 0)
    {
        streamId = 0;
    }
    else if (strncmp(fFileName, "live_stream2", 12) == 0)
    {
        streamId = 1;
    }
    else if (strncmp(fFileName, "live_stream3", 12) == 0)
    {
        streamId = 2;
    }
    else if (strncmp(fFileName, "live_stream4", 12) == 0)
    {
        streamId = 3;
    }
    else
    {
#if defined( USE_V3_3_CODE )
        // Create the video source:
        ByteStreamFileSource* fileSource
        = ByteStreamFileSource::createNew(envir(), fFileName);
        if (fileSource == NULL) return NULL;
        fFileSize = fileSource->fileSize();

        // Create a framer for the Video Elementary Stream:
        if (fEncType == IAV_ENCODE_H264)
        {
            return MyH264VideoStreamFramer::createNew(envir(), fileSource);
        }
        else if (fEncType == IAV_ENCODE_MJPEG)
        {
            return NULL; //not realized
        }
        else
        {
            return NULL;
        }
#else
        // Create the video source:
        ByteStreamFileSource* fileSource
        = ByteStreamFileSource::createNew(envir(), fFileName);
        if (fileSource == NULL) return NULL;
        fFileSize = fileSource->fileSize();

        return MyH264VideoStreamFramer::createNew(envir(), fileSource);
#endif
    }

#if defined( USE_V3_3_CODE )
    if (fEncType == IAV_ENCODE_H264)
    {
        return MyH264VideoStreamFramer::createNew(envir(), streamId);
    }
    else if (fEncType == IAV_ENCODE_MJPEG)
    {
        int jpegQuality = getJpegQ(streamId);
        if ( jpegQuality < 0)
        {
            return NULL;
        }
        return MyJPEGVideoSource::createNew(envir(), streamId, jpegQuality);
    }
    else
    {
        return NULL;
    }
#else
    return MyH264VideoStreamFramer::createNew( envir(), streamId, fIPCMediaDataDispatchServerPort, fIPCMediaDataDispatchClientPort );
#endif
}

RTPSink* H264VideoFileServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
                   unsigned char rtpPayloadTypeIfDynamic,
                   FramedSource* /*inputSource*/)
{
#if defined( USE_V3_3_CODE )
    if (fEncType == IAV_ENCODE_H264)
        return MyH264VideoRTPSink::createNew(envir(), rtpGroupsock,96 ,0X42,"h264");
    else if (fEncType == IAV_ENCODE_MJPEG)
        return MyJPEGVideoRTPSink::createNew(envir(), rtpGroupsock);
    else
        return NULL;
#else
    return MyH264VideoRTPSink::createNew(envir(), rtpGroupsock,96 ,0X42,"h264");
#endif
}


char const*
H264VideoFileServerMediaSubsession::sdpLines()
{
    int encode_type = getEncType();
    if (encode_type == -1)
    {
        return NULL;
    }
    if (fSDPLines == NULL || fEncType != encode_type)
    {
        fEncType = encode_type;
        // We need to construct a set of SDP lines that describe this
        // subsession (as a unicast stream).  To do so, we first create
        // dummy (unused) source and "RTPSink" objects,
        // whose parameters we use for the SDP lines:
        unsigned estBitrate;
        FramedSource* inputSource = createNewStreamSource(0, estBitrate);
        if (inputSource == NULL) return NULL; // file not found

        struct in_addr dummyAddr;
        dummyAddr.s_addr = 0;
        Groupsock dummyGroupsock(envir(), dummyAddr, 0, 0);
        unsigned char rtpPayloadType = 96 + trackNumber()-1; // if dynamic
        RTPSink* dummyRTPSink
        = createNewRTPSink(&dummyGroupsock, rtpPayloadType, inputSource);
        setSDPLinesFromRTPSink(dummyRTPSink, inputSource, estBitrate);
        Medium::close(dummyRTPSink);
        closeStreamSource(inputSource);
    }
    return fSDPLines;
}

int H264VideoFileServerMediaSubsession::getEncType()
{
    int stream_id = 0;
    if (strncmp(fFileName, "live_stream1", 12) == 0)
    {
        stream_id = 0;
    }
    else if (strncmp(fFileName, "live_stream2", 12) == 0)
    {
        stream_id = 1;
    }
    else if (strncmp(fFileName, "live_stream3", 12) == 0)
    {
        stream_id = 2;
    }
    else if (strncmp(fFileName, "live_stream4", 12) == 0)
    {
        stream_id = 3;
    }
    else
    {
        return -1;
    }

    READ_MYSELF( stream_id );
#if defined( USE_V3_3_CODE )
    int fd_iav;
    if ((fd_iav = ::open("/dev/iav", O_RDWR, 0)) < 0)
    {
        perror("/dev/iav");
        return -1;
    }

    iav_encode_format_ex_t format;
    format.id = 1<<stream_id;

    if (ioctl(fd_iav,IAV_IOC_GET_ENCODE_FORMAT_EX, &format ) <0 )
    {
        perror("IAV_IOC_GET_ENCODE_FORMAT_EX");
        return -1;
    }
    ::close(fd_iav);

    return format.encode_type;
#else
    return 0;
#endif
}

int H264VideoFileServerMediaSubsession::getJpegQ( int streamID)
{
#if defined( USE_V3_3_CODE )
    int fd_iav;
    if ((fd_iav = ::open("/dev/iav", O_RDWR, 0)) < 0)
    {
        perror("/dev/iav");
        return -1;
    }
    iav_jpeg_config_ex_t jpeg_config;
    jpeg_config.id =  (1<< streamID);
    if (ioctl(fd_iav, IAV_IOC_GET_JPEG_CONFIG_EX, &jpeg_config) < 0)
    {
        perror("IAV_IOC_GET_JPEG_CONFIG_EX");
        return -1;
    }
    ::close(fd_iav);
    return jpeg_config.quality;
#else
    return 0;
#endif
}
