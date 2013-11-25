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
import xunit.extlib.sdksock

def OptsHasName(opts,name):
	if not hasattr(opts,name):
		return 0
	if getattr(opts,name) is None:
		return 0

	return 1


def SysCtlPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkSysCtlSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('session id %s'%(sesid))
	if OptsHasName(opts,'resethard'):
		ssock.ResetHard()
	if OptsHasName(opts,'reboot'):
		ssock.Reboot()
	logging.info('success')
	return


def SysCtlSessionId(host,port,sesid,opts):
	ssock = xunit.extlib.sdksock.SdkSysCtlSock(host,port)
	sesid = ssock.LoginSessionId(sesid)
	logging.info('session id %s'%(sesid))
	if OptsHasName(opts,'resethard'):
		ssock.ResetHard()
	if OptsHasName(opts,'reboot'):
		ssock.Reboot()
	logging.info('success')
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

	if OptsHasName(opts,'reboot') :
		return

	if OptsHasName(opts,'resethard'):
		return

	Usage(args,3,'please set sysctl command')
	return

def SetArgsTrue(args,name):
	args.add_option('--%s'%(name),action='store_true',dest='%s'%(name),default=None,help='to set %s'%(name))
	return

def main():
	args = OptionParser()
	args.add_option('-v','--verbose',action='count',dest='verbose',help='verbose mode if -vvv debug most')
	args.add_option('-H','--host',action='store',dest='host',nargs=1,help='specify the host')
	args.add_option('-p','--port',action='store',type='int',dest='port',nargs=1,help='specify the port default is 23')
	args.add_option('-u','--user',action='store',dest='user',nargs=1,help='specify username')
	args.add_option('-P','--pass',action='store',dest='password',nargs=1,help='specify password')
	args.add_option('-s','--session',action='store',dest='session',type='int',nargs=1,help='specify sessionid if has this ,would not login with user password')

	SetArgsTrue(args,'reboot')
	SetArgsTrue(args,'resethard')
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)

	
	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	logging.info('nargs %s'%(repr(nargs)))
	if options.session :
		SysCtlSessionId(options.host,options.port,options.session,options)
	else:
		SysCtlPassword(options.host,options.port,options.user,options.password,options)	
	return

if __name__ == '__main__':
	main()
	

