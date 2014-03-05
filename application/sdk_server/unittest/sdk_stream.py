#! python

import os
import re
import random
import sys
import time
from optparse import OptionParser
import logging
import subprocess
import random
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__),'..','..','..','..')))
import xunit.config
import xunit.utils.exptel
import xunit.case
import xunit.suite
import xunit.result
import xunit.logger
import xunit.extlib.nfsmap
import xunit.extlib.cfgexptel
import xunit.extlib.sdksock

def OptsHasName(opts,name):
	if not hasattr(opts,name):
		return 0
	if getattr(opts,name) is None:
		return 0

	return 1


def __PlayStream(ssock,opts):
	lastvidx = None
	lastaidx = None
	cpackets = 0
	apackets = 0
	sleeptime = None
	if opts.sleeptime is not None:		
		sleeptime = float(opts.sleeptime)
		logging.info('sleeptime %s'%(sleeptime))
		random.seed(time.time())
	vfile = None
	afile = None
	lastaudiotime=None
	curaudiotime=None
	lastvideotime=None
	curvideotime=None
	if OptsHasName(opts,'audiodump'):
		afile = open(opts.audiodump,'w+b')
	if OptsHasName(opts,'videodump'):
		vfile = open(opts.videodump,'w+b')
	try:
		while opts.count == 0 or cpackets < opts.count:
			packtype = ssock.GetStreamPacket()
			if packtype != xunit.extlib.sdkproto.pack.GMIS_PROTOCOL_TYPE_MEDIA_DATA:
				continue

			frametype = ssock.GetFrameType()

			if frametype == 'A':
				curidx = ssock.GetStreamIdx()
				if lastaidx is not None:
					if lastaidx == 0xffffffff and curidx != 0 :
						logging.error('audio frame lastidx %d curidx %d'%(lastaidx,curidx))
					elif curidx != (lastaidx + 1):
						logging.error('audio frame lastidx %d curidx %d'%(lastaidx,curidx))
				lastaidx = curidx
				curaudiotime = time.time()
				#sys.stdout.write('audio[%d] %f\n'%(lastaidx,curaudiotime))
				if lastaudiotime is not None:
					if (curaudiotime - lastaudiotime) > 0.15:
						#logging.info('[%d]audio time (%s) (%s) %s'%(curidx,curaudiotime,lastaudiotime,(curaudiotime - lastaudiotime)))
						pass
					
					if (curaudiotime - lastaudiotime) > 1.0:
						#logging.info('[%d]audio time (%s) (%s) %s'%(curidx,curaudiotime,lastaudiotime,(curaudiotime - lastaudiotime)))
						#assert(0!=0)
						pass
				lastaudiotime = curaudiotime
				if (lastaidx % 200) == 0:
					logging.info('audio idx %d'%(lastaidx))
				if afile is not None:
					afile.write(ssock.GetStreamData())
			elif frametype == 'I' or frametype == 'P':
				curidx = ssock.GetStreamIdx()
				curvideotime =time.time()
				if lastvidx is not None:
					if lastvidx == 0xffffffff and curidx != 0 and frametype == 'P':
						raise Exception('lastidx 0x%x curidx 0x%x'%(lastvidx,ssock.GetStreamIdx()))
					elif curidx != (lastvidx + 1) and frametype == 'P':
						raise Exception('lastidx 0x%x curidx 0x%x'%(lastvidx,ssock.GetStreamIdx()))
				if lastvidx is not None and (lastvidx + 1) != curidx :
					logging.info('streamtype %s lastidx 0x%x streamidx 0x%x len(%d)'%(frametype,lastvidx,ssock.GetStreamIdx(),len(ssock.GetStreamData())))
					#time.sleep(1.5)
				if lastvidx is not None and (lastvidx % 100) == 0:
					logging.info('receive video [%d]'%(lastvidx))
				if lastvideotime is not None and (curvideotime - lastvideotime) > 0.08:
					logging.info('video(%d) lasttime(%s) curtime(%s)'%(curidx,lastvideotime,curvideotime))
				lastvideotime=curvideotime
				lastvidx = curidx
				cpackets += 1
				if sleeptime is not None:
					r = random.randint(0,100)
					if r <= opts.random:
						time.sleep(sleeptime)
						logging.info('sleep time (%d)'%(cpackets))
				if vfile is not None:
					vfile.write(ssock.GetStreamData())
			else:
				raise Exception('frametype %s'%(frametype))
	finally:
		if vfile is not None:
			vfile.close()
		vfile = None
		if afile is not None:
			afile.close()
		afile = None
	return


def PlayStreamUserPassword(host,port,username,password,opts,streams):
	ssock = xunit.extlib.sdksock.SdkStreamSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.error('[%s:%d]=>[%s:%s]session id %s'%(host,port,username,password,sesid))
	ssock.StartStream(streams)
	__PlayStream(ssock,opts)
	return

def PlayStreamSessionId(host,port,sessionid,opts,streams):
	ssock = xunit.extlib.sdksock.SdkStreamSock(host,port)
	logging.info('will logon for sesid %s'%(sessionid))
	sesid = ssock.LoginSessionId(sessionid)
	logging.info('session id %s'%(sesid))
	ssock.StartStream(streams)
	__PlayStream(ssock,opts)
	return


def Usage(opt,ec,msg=None):
	fp = sys.stderr
	if ec == 0:
		fp = sys.stdout
	if msg :
		fp.write('%s\n'%(msg))
	opt.print_help(fp)
	sys.exit(ec)


def VerifyOptions(opts,args):
	if not hasattr(opts,'host') or opts.host is None:
		Usage(args,3,'please specify host -H')
	if not hasattr(opts,'port') or opts.port is None or opts.port < 0 or opts.port > (1 << 16):
		Usage(args,3,'please specify port by -p')


	if hasattr(opts,'session') and opts.session :
		logging.info('session %s'%(opts.session))
		if opts.session < 0 or opts.session >= ( 1 << 16):
			Usage(args,3,'session >0 and <= %d'%((1 << 16)))
		return
	
	if not hasattr(opts,'user') or opts.user is None:
		Usage(args,3,'please specify user by -u')
	if not hasattr(opts,'password') or opts.password is None:
		Usage(args,3,'please specify password by -P')
	return

def SetArgsStr(args,name):
	args.add_option('--%s'%(name),action='store',nargs=1,type='string',dest='%s'%(name),default=None,help='to set %s'%(name))
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
	args.add_option('-r','--random',action='store',dest='random',type='int',nargs=1,default=20,help='specify random (0-100) probilities of time sleep to emulate the net congestion default 20')
	SetArgsStr(args,'audiodump')
	SetArgsStr(args,'videodump')
	options ,nargs = args.parse_args(sys.argv[1:])
	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(asctime)s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	VerifyOptions(options,args)

	if len(nargs) == 0 :
		Usage(args,3,'please specify one streamids')
	

	logging.info('nargs %s'%(repr(nargs)))
	if options.session :
		PlayStreamSessionId(options.host,options.port,options.session,options,nargs)
	else:
		PlayStreamUserPassword(options.host,options.port,options.user,options.password,options,nargs)

	
	return

if __name__ == '__main__':
	main()
