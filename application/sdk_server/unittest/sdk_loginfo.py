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
import xunit.extlib.sdksock
from xunit.extlib.sdkproto import loginfo as loginfo

def OptsHasName(opts,name):
	if not hasattr(opts,name):
		return 0
	if getattr(opts,name) is None:
		return 0

	return 1


def SetArgsInt(args,name):
	args.add_option('--%s'%(name),action='store',nargs=1,type='int',dest='%s'%(name),default=None,help='to set %s'%(name))
	return

def SetArgsStr(args,name):
	args.add_option('--%s'%(name),action='store',nargs=1,type='string',dest='%s'%(name),default=None,help='to set %s'%(name))
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
	else:		
		if not hasattr(opts,'user') or opts.user is None:
			Usage(args,3,'please specify user by -u')
		if not hasattr(opts,'password') or opts.password is None:
			Usage(args,3,'please specify password by -P')

	
	return 


def __GetLogInfo(ssock,opts):
	search = loginfo.LogInfoSearch()

	search.SelectMode(opts.selectmode)
	search.MajorType(opts.majortype)
	search.MinorType(opts.minortype)
	search.StartTime(opts.starttime)
	search.StopTime(opts.stoptime)
	search.Offset(opts.offset)
	search.MaxNum(opts.maxnum)
	logging.info("search %s"%(repr(search)))

	loginfos = ssock.QuerySearch(search)
	logging.info('query %s'%(repr(search)))
	logging.info('result %s'%(repr(loginfos)))
	i = 0
	sys.stdout.write('totalnum   (%d)'%(len(loginfos)))
	for info in loginfos:
		sys.stdout.write('++++++++%d+++++++\n'%(i))
		sys.stdout.write('%s'%(repr(info)))
		sys.stdout.write('--------%d-------\n'%(i))
		i += 1
	return

def GetLogInfoSession(host,port,sesid,opts):
	ssock = xunit.extlib.sdksock.SdkLogInfoSock(host,port)
	ssock.LoginSessionId(sessid)
	return __GetLogInfo(ssock,opts)

def GetLogInfoUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkLogInfoSock(host,port)
	ssock.LoginUserPass(username,password)
	return __GetLogInfo(ssock,opts)

def main():
	args = OptionParser()
	args.add_option('-v','--verbose',action='store_true',dest='verbose',help='verbose mode')
	args.add_option('-H','--host',action='store',dest='host',nargs=1,help='specify the host')
	args.add_option('-p','--port',action='store',type='int',dest='port',nargs=1,help='specify the port default is 23')
	args.add_option('-u','--user',action='store',dest='user',nargs=1,help='specify username')
	args.add_option('-P','--pass',action='store',dest='password',nargs=1,help='specify password')
	args.add_option('-s','--session',action='store',dest='session',type='int',nargs=1,help='specify sessionid if has this ,would not login with user password')
	SetArgsInt(args,'selectmode')
	SetArgsInt(args,'majortype')
	SetArgsInt(args,'minortype')
	
	SetArgsStr(args,'starttime')
	SetArgsStr(args,'stoptime')
	SetArgsInt(args,'offset')
	SetArgsInt(args,'maxnum')
	
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)

	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(asctime)s %(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	if options.session :
		GetLogInfoSession(options.host,options.port,options.session,options)
	else:
		GetLogInfoUserPassword(options.host,options.port,options.user,options.password,options)
	return

if __name__ == '__main__':
	main()

