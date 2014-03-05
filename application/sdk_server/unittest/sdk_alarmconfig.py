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
from xunit.extlib.sdkproto import alarmconfig as alarmconfig

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

	if OptsHasName(opts,'getcfg'):
		return

	if not OptsHasName(opts,'alarmid'):
		Usage(args,3,'not specify alarmid for setcfg')
	if not OptsHasName(opts,'index'):
		Usage(args,3,'not specify index for setcfg')

	if OptsHasName(opts,'alarminextinfoionum'):
		return
	if OptsHasName(opts,'alarminextinfooperatecmd'):
		return
	if OptsHasName(opts,'alarminextinfooperateseqnum'):
		return
	if OptsHasName(opts,'alarminextinfoptzdelaytime'):
		return
	if OptsHasName(opts,'alarminextinfodelaytime'):
		return
	if OptsHasName(opts,'alarminenableflag'):
		return
	if OptsHasName(opts,'alarmininputnumber'):
		return

	if OptsHasName(opts,'alarminname'):
		return
	if OptsHasName(opts,'alarminchecktime'):
		return
	if OptsHasName(opts,'alarminnormalstatus'):
		return
	if OptsHasName(opts,'alarminlinkalarmstrategy'):
		return
	if OptsHasName(opts,'alarmoutenableflag'):
		return
	if OptsHasName(opts,'alarmoutoutputnumber'):
		return
	if OptsHasName(opts,'alarmoutname'):
		return
	if OptsHasName(opts,'alarmoutnormalstatus'):
		return
	if OptsHasName(opts,'alarmoutdelaytime'):
		return
	if OptsHasName(opts,'alarmeventextinfoionum'):
		return
	if OptsHasName(opts,'alarmeventextinfooperatecmd'):
		return
	if OptsHasName(opts,'alarmeventextinfooperateseqnum'):
		return
	if OptsHasName(opts,'alarmeventextinfoptzdelaytime'):
		return
	if OptsHasName(opts,'alarmeventextinfodelaytime'):
		return
	if OptsHasName(opts,'alarmeventpirdetectdatasensitive'):
		return
	if OptsHasName(opts,'alarmeventalarmid'):
		return
	if OptsHasName(opts,'alarmeventenableflag'):
		return
	if OptsHasName(opts,'alarmeventchecktime'):
		return
	if OptsHasName(opts,'alarmeventlinkalarmstrategy'):
		return

	Usage(args,3,'must specify a member to setcfg')	
	return 
def __GetConfigsInner(ssock,opts):
	getconfig = alarmconfig.AlarmConfig()
	getconfig.AlarmId(opts.alarmid)
	getconfig.Index(opts.index)
	return ssock.GetAlarmConfig(getconfig)

def __GetConfigs(ssock,opts):
	configs = __GetConfigsInner(ssock,opts)
	i = 0
	for c in configs:
		sys.stdout.write('----------%d-------\n'%(i))
		sys.stdout.write('%s'%(repr(c)))
	return

def __SetConfigs(ssock,opts):
	configs = __GetConfigsInner(ssock,opts)

	i = 0
	for c in configs:
		sys.stdout.write('-------read[%d]-------\n'%(i))
		sys.stdout.write('%s'%(repr(c)))
		i += 1
	setconfigs = []
	for c in configs:
		if isinstance(c,alarmconfig.AlarmInConfig):
			c.EnableFlag(opts.alarminenableflag)
			c.InputNumber(opts.alarmininputnumber)
			c.Name(opts.alarminname)
			c.CheckTime(opts.alarminchecktime)
			c.NormalStatus(opts.alarminnormalstatus)
			c.LinkAlarmStrategy(opts.alarminlinkalarmstrategy)
			c.ExtInfoIoNum(opts.alarminextinfoionum)
			c.ExtInfoOperateCmd(opts.alarminextinfooperatecmd)
			c.ExtInfoOperateSeqNum(opts.alarminextinfooperateseqnum)
			c.ExtInfoPtzDelayTime(opts.alarminextinfoptzdelaytime)
			c.ExtInfoDelayTime(opts.alarminextinfodelaytime)
		elif isinstance(c,alarmconfig.AlarmOutConfig):
			c.EnableFlag(opts.alarmoutenableflag)
			c.OutputNumber(opts.alarmoutoutputnumber)
			c.Name(opts.alarmoutname)
			c.NormalStatus(opts.alarmoutnormalstatus)
			c.DelayTime(opts.alarmoutdelaytime)
			
		elif isinstance(c,alarmconfig.AlarmEventConfig):
			c.AlarmId(opts.alarmeventalarmid)
			c.EnableFlag(opts.alarmeventenableflag)
			c.CheckTime(opts.alarmeventchecktime)
			c.LinkAlarmStrategy(opts.alarmeventlinkalarmstrategy)
			c.UnionSensitive(opts.alarmeventpirdetectdatasensitive)
			c.ExtInfoIoNum(opts.alarmeventextinfoionum)
			c.ExtInfoOperateCmd(opts.alarmeventextinfooperatecmd)
			c.ExtInfoOperateSeqNum(opts.alarmeventextinfooperateseqnum)
			c.ExtInfoPtzDelayTime(opts.alarmeventextinfoptzdelaytime)
			c.ExtInfoDelayTime(opts.alarmeventextinfodelaytime)
		else:
			raise Exception('not recognize config')
		setconfigs.append(c)
	i = 0
	for c in setconfigs:
		sys.stdout.write('-----change[%d]-------\n'%(i))
		sys.stdout.write('%s'%(repr(c)))
		i += 1
	ssock.SetAlarmConfig(setconfigs)
	configs = __GetConfigsInner(ssock,opts)
	i = 0
	for c in configs:
		sys.stdout.write('-----reread[%d]-------\n'%(i))
		sys.stdout.write('%s'%(repr(c)))
		i += 1
	return

def GetConfigUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkAlarmConfigSock(host,port)
	ssock.LoginUserPass(username,password)
	return __GetConfigs(ssock,opts)
def GetConfigSessionId(host,port,sessid,opts):
	ssock = xunit.extlib.sdksock.SdkAlarmConfigSock(host,port)
	ssock.LoginSessionId(sessid)
	return __GetConfigs(ssock,opts)

def SetConfigUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkAlarmConfigSock(host,port)
	ssock.LoginUserPass(username,password)
	return __SetConfigs(ssock,opts)
def SetConfigSessionId(host,port,sessid,opts):
	ssock = xunit.extlib.sdksock.SdkAlarmConfigSock(host,port)
	ssock.LoginSessionId(sessid)
	return __SetConfigs(ssock,opts)

	

def main():
	args = OptionParser()
	args.add_option('-v','--verbose',action='store_true',dest='verbose',help='verbose mode')
	args.add_option('-H','--host',action='store',dest='host',nargs=1,help='specify the host')
	args.add_option('-p','--port',action='store',type='int',dest='port',nargs=1,help='specify the port default is 23')
	args.add_option('-u','--user',action='store',dest='user',nargs=1,help='specify username')
	args.add_option('-P','--pass',action='store',dest='password',nargs=1,help='specify password')
	args.add_option('-s','--session',action='store',dest='session',type='int',nargs=1,help='specify sessionid if has this ,would not login with user password')
	args.add_option('-g','--getcfg',action='store_true',dest='getcfg',default=None,help='specify to get cfg')
	SetArgsInt(args,'alarmid')
	SetArgsInt(args,'index')
	SetArgsInt(args,'alarminextinfoionum')
	SetArgsInt(args,'alarminextinfooperatecmd')
	SetArgsInt(args,'alarminextinfooperateseqnum')
	SetArgsInt(args,'alarminextinfoptzdelaytime')
	SetArgsInt(args,'alarminextinfodelaytime')
	SetArgsInt(args,'alarminenableflag')
	SetArgsInt(args,'alarmininputnumber')
	SetArgsStr(args,'alarminname')
	SetArgsInt(args,'alarminchecktime')
	SetArgsInt(args,'alarminnormalstatus')
	SetArgsInt(args,'alarminlinkalarmstrategy')
	SetArgsInt(args,'alarmoutenableflag')
	SetArgsInt(args,'alarmoutoutputnumber')
	SetArgsStr(args,'alarmoutname')
	SetArgsInt(args,'alarmoutnormalstatus')
	SetArgsInt(args,'alarmoutdelaytime')
	SetArgsInt(args,'alarmeventextinfoionum')
	SetArgsInt(args,'alarmeventextinfooperatecmd')
	SetArgsInt(args,'alarmeventextinfooperateseqnum')
	SetArgsInt(args,'alarmeventextinfoptzdelaytime')
	SetArgsInt(args,'alarmeventextinfodelaytime')
	SetArgsInt(args,'alarmeventpirdetectdatasensitive')
	SetArgsInt(args,'alarmeventalarmid')
	SetArgsInt(args,'alarmeventenableflag')
	SetArgsInt(args,'alarmeventchecktime')
	SetArgsInt(args,'alarmeventlinkalarmstrategy')
	
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)

	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(asctime)s %(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	if options.session :
		if options.getcfg is not None:
			GetConfigSessionId(options.host,options.port,options.session,options)
		else:
			SetConfigSessionId(options.host,options.port,options.session,options)
	else:
		if options.getcfg is not None:
			GetConfigUserPassword(options.host,options.port,options.user,options.password,options)
		else:
			SetConfigUserPassword(options.host,options.port,options.user,options.password,options)
	return

if __name__ == '__main__':
	main()


