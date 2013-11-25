#! python

import os
import re
import random
import sys
import time
from optparse import OptionParser
import logging
import subprocess
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


def LoginOneSession(host,port,sesid,timeout=10):
	ssock = xunit.extlib.sdksock.SdkSock(host,port)
	getsesid = ssock.LoginSessionId(sesid)
	assert(getsesid == sesid)
	ssock.CloseSocket()
	del ssock
	ssock = None
	return
	

def LoginSession(host,port,sesid,timeout=10):
	logging.error('start %d'%(time.time()))
	while True:
		LoginOneSession(host,port,sesid,timeout)
		time.sleep(5)
	return 
	

def LoginUserName(host,port,username,password,count=20):
	sockarr = []
	for i in xrange(count):
		ssock = xunit.extlib.sdksock.SdkSock(host,port)
		sesid = ssock.LoginUserPass(username,password)
		logging.error('[%d][%s:%d]=>[%s:%s]session id %s'%(i,host,port,username,password,sesid))
		sockarr.append(ssock)

	for s in sockarr:
		s.CloseSocket()

	sockarr = []
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
	args.add_option('-c','--count',action='store',type='int',dest='count',nargs=1,default=10,help='set timeout value default is 5')
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)
	
	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	if options.session :
		LoginSession(options.host,options.port,options.session,options.count)
	else:
		LoginUserName(options.host,options.port,options.user,options.password,options.count)

	
	return

if __name__ == '__main__':
	logging.basicConfig(level=logging.INFO,format="%(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")
	main()
	
