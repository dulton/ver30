import os
import re
import random
import sys
import time
from optparse import OptionParser
import logging
import thread
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__),'..','..','..','..')))
import xunit.extlib.sdksock
import xunit.extlib.sdkproto as sdkproto

def OptsHasName(opts,name):
	if not hasattr(opts,name):
		return 0
	if getattr(opts,name) is None:
		return 0
	return 1

threadrunning=1
threadexit=0
MAX_INTS=0xffffffff
MAX_LONGLONGS=((1<<64)-1)

def ReceiveThread(ssock,opts):
	audiofi = None
	audiotime = 2.0
	lastidx = None
	global threadrunning
	global threadexit
	try:
		if OptsHasName(opts,'audiodump'):
			audiofi = open(opts.audiodump,'w+b')
		if OptsHasName(opts,'audiotimeout'):
			audiotime = float(opts.audiotimeout)
		while threadrunning :
			data = ssock.ReceiveData(audiotime)
			framepack = ssock.GetReceiveHdr()
			if lastidx is not None:
				expectidx = lastidx
				expectidx += 1
				if expectidx > MAX_INTS:
					expectidx = 0
				if framepack.FrameId() != expectidx:
					logging.error('lastidx (%d) new (%d)'%(lastidx,framepack.FrameId()))
			lastidx = framepack.FrameId()
			if (lastidx % 50) == 0:
				logging.info('idx %d'%(lastidx))
			if audiofi is not None:
				audiofi.write(data)
	finally:
		if audiofi is not None:
			audiofi.close()
		audiofi = None
		threadexit = 1
	return

def __SendDataOut(ssock,framepack,opts):
	audiofo = None	
	audiootime = 0.02
	audioosize = 160
	lastpts = 0
	steppts = 1800
	curoff=0
	mostcount = 0
	sendcount = 0
	global threadrunning
	global threadexit
	try:
		audiofo = open(opts.audioout,'r+b')
		if OptsHasName(opts,'audiootime'):
			audiootime = float(opts.audiootime)
		if OptsHasName(opts,'audioosize'):
			audioosize = int(audioosize)
		if OptsHasName(opts,'steppts'):
			steppts = int(opts.steppts)
		if OptsHasName(opts,'audiootime'):
			audiootime = float(opts.audiootime)
		if OptsHasName(opts,'count'):
			mostcount = int(opts.count)
		audiofo.seek(0,2)
		fosize = audiofo.tell()
		audiofo.seek(0,0)
		curpts = framepack.Pts()
		curframeid = framepack.FrameId()
		while mostcount == 0 or sendcount < mostcount:
			if (curoff + audioosize) > fosize:
				audiofo.seek(0,0)
				curoff = 0
			sbuf = audiofo.read(audioosize)
			curoff += audioosize
			ssock.SendData(framepack,sbuf)
			curpts += steppts
			if curpts > MAX_LONGLONGS:
				curpts %= MAX_LONGLONGS			
			curframeid += 1
			if curframeid > MAX_INTS:
				curframeid %= MAX_INTS
			framepack.Pts(curpts)
			framepack.FrameId(curframeid)
			sendcount += 1
			time.sleep(audiootime)
			if (sendcount % 100) == 0:
				logging.info('send %d'%(sendcount))
			
	except:
		if audiofo is not None:
			audiofo.close()
		audiofo = None
		threadrunning = 0
		while threadexit == 0:
			time.sleep(0.1)
		
def __HandleAudioDual(ssock,opts):
	# now we should start with function name
	framepack = sdkproto.audiodual.AudioFramePack()
	startreq = sdkproto.audiodual.StartTalkRequest()
	resp = ssock.StartAudioDual(startreq)
	logging.info('====================\n%s====================\n'%(str(resp)))
	thread.start_new_thread(ReceiveThread,(ssock,opts,))
	__SendDataOut(ssock,framepack,opts)
	return 


def AudioDualUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkAudioDualSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.error('[%s:%d]=>[%s:%s]session id %s'%(host,port,username,password,sesid))
	return __HandleAudioDual(ssock,opts)

def AudioDualSessionId(host,port,sessionid,opts):
	ssock = xunit.extlib.sdksock.SdkAudioDualSock(host,port)
	sesid = ssock.LoginSessionId(sessionid)
	logging.info('session id %s'%(sesid))
	return __HandleAudioDual(ssock,opts)


def SetArgsInt(args,name,defval=None):
	if defval is not None:
		args.add_option('--%s'%(name),action='store',nargs=1,type='int',dest='%s'%(name),default=defval,help='to set %s default %s'%(name,repr(defval)))
	else:
		args.add_option('--%s'%(name),action='store',nargs=1,type='int',dest='%s'%(name),default=defval,help='to set %s default None'%(name))

def SetArgsFloat(args,name,defval=None):
	if defval is not None:
		args.add_option('--%s'%(name),action='store',nargs=1,type='float',dest='%s'%(name),default=defval,help='to set %s default %s'%(name,repr(defval)))
	else:
		args.add_option('--%s'%(name),action='store',nargs=1,type='float',dest='%s'%(name),default=defval,help='to set %s default None'%(name))


def SetArgsStr(args,name,defval=None):
	if defval is not None:
		args.add_option('--%s'%(name),action='store',nargs=1,type='string',dest='%s'%(name),default=defval,help='to set %s default %s'%(name,repr(defval)))
	else:
		args.add_option('--%s'%(name),action='store',nargs=1,type='string',dest='%s'%(name),default=defval,help='to set %s default None'%(name))

def Usage(opt,ec,msg=None):
	fp = sys.stderr
	if ec == 0:
		fp = sys.stdout
	if msg :
		fp.write('%s\n'%(msg))
	opt.print_help(fp)
	sys.exit(ec)


def VerifyOptions(opts,args):
	if not OptsHasName(opts,'audioout'):
		Usage(args,3,'please specify audioout')
	if not OptsHasName(opts,'host'):
		Usage(args,3,'please specify host -H')
	if not OptsHasName(opts,'port') or opts.port < 0 or opts.port > (1 << 16):
		Usage(args,3,'please specify port by -p')


	if OptsHasName(opts,'session') :
		logging.info('session %s'%(opts.session))
		if opts.session < 0 or opts.session >= ( 1 << 16):
			Usage(args,3,'session >0 and <= %d'%((1 << 16)))
		return
	
	if not OptsHasName(opts,'user'):
		Usage(args,3,'please specify user by -u')
	if not OptsHasName(opts,'password'):
		Usage(args,3,'please specify password by -P')

	return

def main():
	args = OptionParser()
	args.add_option('-v','--verbose',action='count',dest='verbose',help='verbose mode if -vvv debug most')
	args.add_option('-H','--host',action='store',dest='host',nargs=1,help='specify the host')
	args.add_option('-p','--port',action='store',type='int',dest='port',nargs=1,help='specify the port default is 23')
	args.add_option('-u','--user',action='store',dest='user',nargs=1,help='specify username')
	args.add_option('-P','--pass',action='store',dest='password',nargs=1,help='specify password')
	args.add_option('-s','--session',action='store',dest='session',type='int',nargs=1,help='specify sessionid if has this ,would not login with user password')
	args.add_option('-c','--count',action='store',dest='count',type='int',nargs=1,default=0,help='specify count packets default is 0 means no limit')
	args.add_option('-t','--sleeptime',action='store',dest='sleeptime',type='float',nargs=1,default=None,help='specify sleeptime default is None ,no sleep')
	SetArgsStr(args,'audioout')
	SetArgsStr(args,'audiodump')
	SetArgsFloat(args,'audiotimeout')
	SetArgsFloat(args,'audiootime')
	SetArgsInt(args,'audioosize')
	SetArgsInt(args,'steppts')
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)

	
	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(asctime)s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	logging.info('nargs %s'%(repr(nargs)))
	if options.session :
		AudioDualSessionId(options.host,options.port,options.session,options)
	else:
		AudioDualUserPassword(options.host,options.port,options.user,options.password,options)

	
	return

if __name__ == '__main__':
	main()
	

