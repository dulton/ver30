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
from xunit.extlib.sdkproto import daynight as daynight

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
def SetArgsCallback(args,name,callfn):
	args.add_option('--%s'%(name),action='callback',type="string",callback=callfn,dest='%s'%(name),default=None,help='to set %s'%(name))
	return

def SchedStartTimeCallBack(option,opt_str,value,parser):
	if opt_str == '--schedstarttime' :
		if OptsHasName(parser.values,'schedstarttime'):
			v = int(value)
			parser.values.schedstarttime.append()
		else:
			parser.values.schedstarttime = []
			v = int(value)
			parser.values.schedstarttime.append(v)
	else:
		raise Exception('opt_str(%s) not schedstarttime'%(opt_str))
	return

def SchedStopTimeCallBack(option,opt_str,value,parser):
	if opt_str == '--schedstoptime' :
		if OptsHasName(parser.values,'schedstoptime'):
			v = int(value)
			parser.values.schedstoptime.append()
		else:
			parser.values.schedstoptime = []
			v = int(value)
			parser.values.schedstoptime.append(v)
	else:
		raise Exception('opt_str(%s) not schedstoptime'%(opt_str))
	return

def SchedEnableCallBack(option,opt_str,value,parser):
	if opt_str == '--schedenable' :
		if OptsHasName(parser.values,'schedenable'):
			v = int(value)
			parser.values.schedenable.append()
		else:
			parser.values.schedenable = []
			v = int(value)
			parser.values.schedenable.append(v)
	else:
		raise Exception('opt_str(%s) not schedenable'%(opt_str))
	return

def SchedIdxCallBack(option,opt_str,value,parser):
	if opt_str == '--schedidx' :
		if OptsHasName(parser.values,'schedidx'):
			v = int(value)
			parser.values.schedidx.append()
		else:
			parser.values.schedidx = []
			v = int(value)
			parser.values.schedidx.append(v)
	else:
		raise Exception('opt_str(%s) not schedidx'%(opt_str))
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
	hasvalue = 0
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

	if OptsHasName(opts,'getcfg'):
		return

	if OptsHasName(opts,'schedenable') or OptsHasName(opts,'schedidx') \
		or OptsHasName(opts,'schedstarttime') or\
		OptsHasName(opts,'schedstoptime'):
		if not OptsHasName(opts,'schedenable'):
			Usage(args,3,'must specify schedenable with');
		if not OptsHasName(opts,'schedidx'):
			Usage(args,3,'must specify schedidx with');
		if not OptsHasName(opts,'schedstarttime'):
			Usage(args,3,'must specify schedstarttime with');
		if not OptsHasName(opts,'schedstoptime'):
			Usage(args,3,'must specify schedstoptime with');
		if len(opts.schedidx) != len(opts.schedenable):
			Usage(args,3,'schedidx size(%d) != schedenable size(%d)'%(len(opts.schedidx),len(opts.schedenable)));
		if len(opts.schedidx) != len(opts.schedstarttime):
			Usage(args,3,'schedidx size(%d) != schedstarttime size(%d)'%(len(opts.schedidx),len(opts.schedstarttime)));
		if len(opts.schedidx) != len(opts.schedstoptime):
			Usage(args,3,'schedidx size(%d) != schedstoptime size(%d)'%(len(opts.schedidx),len(opts.schedstoptime)));
		hasvalue = 1
	if  OptsHasName(opts,'mode'):
		return
	if OptsHasName(opts,'durationtime'):
		return
	if OptsHasName(opts,'daytonightthr'):
		return
	if OptsHasName(opts,'nighttodaythr'):
		return
	return 


def __GetDayNightInner(ssock,opts):
	return ssock.GetDayNight()

def __GetDayNight(ssock,opts):
	dn = __GetDayNightInner(ssock,opts)
	sys.stdout.write('%s'%(repr(dn)))
	return

def __SetDayNight(ssock,opts):
	dn = __GetDayNightInner(ssock,opts)
	sys.stdout.write('-----------origine----------\n')
	sys.stdout.write('%s'%(repr(dn)))
	dn.Mode(opts.mode)
	dn.DurationTime(opts.durationtime)
	dn.DayToNightThr(opts.daytonightthr)
	dn.NightToDayThr(opts.nighttodaythr)
	if OptsHasName(opts,'schedidx'):
		for i in xrange(len(opts.schedidx)):
			dn.SchedEnableIdx(opts.schedidx[i],opts.schedenable[i])
			dn.SchedStartTimeIdx(opts.schedidx[i],opts.schedstarttime[i])
			dn.SchedStopTimeIdx(opts.schedidx[i],opts.schedstoptime[i])
	
	sys.stdout.write('-----------change----------\n')
	sys.stdout.write('%s'%(repr(dn)))
	ssock.SetDayNight(dn)
	dn = __GetDayNightInner(ssock,opts)
	sys.stdout.write('-----------reget----------\n')
	sys.stdout.write('%s'%(repr(dn)))
	return

def GetDayNightUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkDayNightSock(host,port)
	ssock.LoginUserPass(username,password)
	return __GetDayNight(ssock,opts)
def GetDayNightSessionId(host,port,sessid,opts):
	ssock = xunit.extlib.sdksock.SdkDayNightSock(host,port)
	ssock.LoginSessionId(sessid)
	return __GetDayNight(ssock,opts)

def SetDayNightUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkDayNightSock(host,port)
	ssock.LoginUserPass(username,password)
	return __SetDayNight(ssock,opts)
def SetDayNightSessionId(host,port,sessid,opts):
	ssock = xunit.extlib.sdksock.SdkDayNightSock(host,port)
	ssock.LoginSessionId(sessid)
	return __SetDayNight(ssock,opts)

def main():
	args = OptionParser()
	args.add_option('-v','--verbose',action='store_true',dest='verbose',help='verbose mode')
	args.add_option('-H','--host',action='store',dest='host',nargs=1,help='specify the host')
	args.add_option('-p','--port',action='store',type='int',dest='port',nargs=1,help='specify the port default is 23')
	args.add_option('-u','--user',action='store',dest='user',nargs=1,help='specify username')
	args.add_option('-P','--pass',action='store',dest='password',nargs=1,help='specify password')
	args.add_option('-s','--session',action='store',dest='session',type='int',nargs=1,help='specify sessionid if has this ,would not login with user password')
	args.add_option('-g','--getcfg',action='store_true',dest='getcfg',default=None,help='specify to get cfg')
	SetArgsInt(args,'mode')
	SetArgsInt(args,'durationtime')
	SetArgsInt(args,'daytonightthr')
	SetArgsInt(args,'nighttodaythr')
	SetArgsCallback(args,'schedstarttime',SchedStartTimeCallBack)
	SetArgsCallback(args,'schedenable',SchedEnableCallBack)
	SetArgsCallback(args,'schedidx',SchedIdxCallBack)
	SetArgsCallback(args,'schedstoptime',SchedStopTimeCallBack)
	
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)

	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(asctime)s %(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	if options.session :
		if options.getcfg is not None:
			GetDayNightSessionId(options.host,options.port,options.session,options)
		else:
			SetDayNightSessionId(options.host,options.port,options.session,options)
	else:
		if options.getcfg is not None:
			GetDayNightUserPassword(options.host,options.port,options.user,options.password,options)
		else:
			SetDayNightUserPassword(options.host,options.port,options.user,options.password,options)
	return

if __name__ == '__main__':
	main()


