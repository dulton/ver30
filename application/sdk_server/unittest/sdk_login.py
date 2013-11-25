#! python

import os
import re
import random
import sys
import time
from optparse import OptionParser
import logging
import subprocess
import time
import traceback
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


def LoginOneSession(host,port,sesid):
	ssock = xunit.extlib.sdksock.SdkSock(host,port)
	getsesid = ssock.LoginHeartBeatTimeout(sesid)
	assert(getsesid == sesid)
	ssock.CloseSocket()
	del ssock
	ssock = None
	return
	

def LoginSession(host,port,sesid,opts):
	fh = sys.stderr
	if opts.write:
		fh = open(opts.write,'w+b')
	fh.write('start at %s\n'%(time.asctime(time.localtime())))
	ssock = None
	if opts.cont :
		ssock = xunit.extlib.sdksock.SdkSock(host,port)
		getsesid = ssock.LoginHeartBeatTimeout(sesid)
		logging.info('connect ok')
	if opts.timeout is not None:
		timeout = float(opts.timeout)
	else:
		timeout = 5.0
	cnt = 0
	try:
		while True:
			LoginOneSession(host,port,sesid)
			if ssock is not None:
				logging.info('sesid %d'%(sesid))
				ssock.LoginHeartBeatTimeout(sesid)
			time.sleep(timeout)			
			cnt += 1
			logging.info('Login[%s:%s] sesid %d cnt(%d)'%(host,port,sesid,cnt))
	except:
		exstr = traceback.format_exc()
		fh.write('Exception %s\n'%(exstr))
		fh.write('EndTime %s\n'%(time.asctime(time.localtime())))
	if ssock is not None:
		ssock.CloseSocket()
	return 
	

def LoginUserName(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.error('[%s:%d]=>[%s:%s]session id %s'%(host,port,username,password,sesid))
	ssock.CloseSocket()
	del ssock
	ssock = None
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

def main():
	args = OptionParser()
	args.add_option('-v','--verbose',action='count',dest='verbose',help='verbose mode if -vvv debug most')
	args.add_option('-H','--host',action='store',dest='host',nargs=1,help='specify the host')
	args.add_option('-p','--port',action='store',type='int',dest='port',nargs=1,help='specify the port default is 23')
	args.add_option('-u','--user',action='store',dest='user',nargs=1,help='specify username')
	args.add_option('-P','--pass',action='store',dest='password',nargs=1,help='specify password')
	args.add_option('-s','--session',action='store',dest='session',type='int',nargs=1,help='specify sessionid if has this ,would not login with user password')
	args.add_option('-t','--timeout',action='store',type='float',dest='timeout',nargs=1,default=5.0,help='set timeout value default is 5')
	args.add_option('-w','--write',action='store',type='str',dest='write',nargs=1,default=None,help='set write file default is stdout')
	args.add_option('-c','--cont',action='store_true',dest='cont',default=0,help='set continue in the login session')
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)
	
	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(asctime)s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	if options.session :
		LoginSession(options.host,options.port,options.session,options)
	else:
		LoginUserName(options.host,options.port,options.user,options.password,options)

	
	return

if __name__ == '__main__':
	logging.basicConfig(level=logging.INFO,format="%(asctime)s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")
	main()
	
