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

def __GetAlarms(ssock,opts):
	count = 0
	needcount = 0
	fp = sys.stdout

	
	if OptsHasName(opts,'count'):
		needcount = opts.count
	stime = time.time()
	etime = stime
	if OptsHasName(opts,'timeout'):
		etime += opts.timeout
	if OptsHasName(opts,'write'):
		fp = open(opts.write,'w+b')
	try:
		while 1:
			alarminfo = ssock.GetAlarmInfo(3)
			ctime = time.time()
			if stime != etime and ctime > etime:
				break
			ssock.SendHeartBeatTimeout()
			if alarminfo is None:
				continue
			count += 1
			for a in alarminfo:
				fp.write('[%d]---------\n%s----------------\n'%(count,repr(a)))
			if needcount != 0 and count >= needcount:
				break
	except KeyboardInterrupt:
		if fp != sys.stdout:
			fp.close()
		return
	if fp != sys.stdout:
		fp.close()		
	return

def GetAlarmSession(host,port,sessid,opts):
	ssock = xunit.extlib.sdksock.SdkAlarmSock(host,port)
	sesid = ssock.LoginSessionAlarm(sessid)
	__GetAlarms(ssock,opts)
	return

def GetAlarmUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkAlarmSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	ssock.ChangeAlarmState()
	__GetAlarms(ssock,opts)
	return

def main():
	args = OptionParser()
	args.add_option('-v','--verbose',action='store_true',dest='verbose',help='verbose mode')
	args.add_option('-H','--host',action='store',dest='host',nargs=1,help='specify the host')
	args.add_option('-p','--port',action='store',type='int',dest='port',nargs=1,help='specify the port default is 23')
	args.add_option('-u','--user',action='store',dest='user',nargs=1,help='specify username')
	args.add_option('-P','--pass',action='store',dest='password',nargs=1,help='specify password')
	args.add_option('-s','--session',action='store',dest='session',type='int',nargs=1,help='specify sessionid if has this ,would not login with user password')
	SetArgsInt(args,'count')
	SetArgsInt(args,'timeout')
	SetArgsStr(args,'write')
	
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)

	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(asctime)s %(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	if options.session :
		GetAlarmSession(options.host,options.port,options.session,options)
	else:
		GetAlarmUserPassword(options.host,options.port,options.user,options.password,options)
	return

if __name__ == '__main__':
	main()


