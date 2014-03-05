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
from xunit.extlib.sdkproto import alarmdeploy as alarmdeploy

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


def StartTimeCallBack(option,opt_str,value,parser):
	if opt_str == '--starttime' :
		if OptsHasName(parser.values,'starttime'):
			v = int(value)
			parser.values.starttime.append(v)
		else:
			parser.values.starttime = []
			v = int(value)
			parser.values.starttime.append(v)
	else:
		raise Exception('opt_str(%s) not starttime'%(opt_str))
	return

def EndTimeCallBack(option,opt_str,value,parser):
	if opt_str == '--endtime' :
		if OptsHasName(parser.values,'endtime'):
			v = int(value)
			parser.values.endtime.append(v)
		else:
			parser.values.endtime = []
			v = int(value)
			parser.values.endtime.append(v)
	else:
		raise Exception('opt_str(%s) not endtime'%(opt_str))
	return

def IdexCallBack(option,opt_str,value,parser):
	if opt_str == '--idex' :
		if OptsHasName(parser.values,'idex'):
			v = int(value)
			parser.values.idex.append(v)
		else:
			parser.values.idex = []
			v = int(value)
			parser.values.idex.append(v)
	else:
		raise Exception('opt_str(%s) not idex'%(opt_str))
	return
def JdexCallBack(option,opt_str,value,parser):
	if opt_str == '--jdex' :
		if OptsHasName(parser.values,'jdex'):
			v = int(value)
			parser.values.jdex.append(v)
		else:
			parser.values.jdex = []
			v = int(value)
			parser.values.jdex.append(v)
	else:
		raise Exception('opt_str(%s) not jdex'%(opt_str))
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
		if not OptsHasName(opts,'scheduleid'):
			Usage(args,3,'getcfg should specify scheduleid')
		if not OptsHasName(opts,'index'):
			Usage(args,3,'getcfg should specify index')
		return
	if not OptsHasName(opts,'scheduleid'):
		Usage(args,3,'setcfg should specify scheduleid')
	if not OptsHasName(opts,'index'):
		Usage(args,3,'setcfg should specify index')

	if not OptsHasName(opts,'starttime'):
		Usage(args,3,'setcfg should specify starttime')
	if not OptsHasName(opts,'endtime'):
		Usage(args,3,'setcfg should specify endtime')
	if not OptsHasName(opts,'idex') or not OptsHasName(opts,'jdex'):
		Usage(args,3,'setcfg should specify idex and jdex')
	if len(opts.starttime) != len(opts.endtime) or \
		len(opts.endtime) != len(opts.idex)  or \
		len(opts.idex)  != len(opts.jdex):
		Usage(args,3,'starttime endtime idex jdex should be equal')
	return 
def __GetDeployInner(ssock,opts):
	getschedule = alarmdeploy.AlarmGetScheduleTimeInfo()
	getschedule.ScheduleId(opts.scheduleid)
	getschedule.Index(opts.index)
	return ssock.GetAlarmDeploy(getschedule)

def __GetDeploy(ssock,opts):
	ad = __GetDeployInner(ssock,opts)
	sys.stdout.write('%s'%(repr(ad)))
	return

def __SetDeploy(ssock,opts):
	ad = __GetDeployInner(ssock,opts)
	sys.stdout.write('-----------origine----------\n')
	sys.stdout.write('%s'%(repr(ad)))
	for i in xrange(len(opts.idex)):
		ad.StartTimeIdx(opts.idex[i],opts.jdex[i],opts.starttime[i])
		ad.EndTimeIdx(opts.idex[i],opts.jdex[i],opts.endtime[i])
	sys.stdout.write('-----------change----------\n')
	sys.stdout.write('%s'%(repr(ad)))
	ssock.SetAlarmDeploy(ad)
	ad = __GetDeployInner(ssock,opts)
	sys.stdout.write('-----------reget----------\n')
	sys.stdout.write('%s'%(repr(ad)))
	return

def GetDeployUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkAlarmDeploySock(host,port)
	ssock.LoginUserPass(username,password)
	return __GetDeploy(ssock,opts)
def GetDeploySessionId(host,port,sessid,opts):
	ssock = xunit.extlib.sdksock.SdkAlarmDeploySock(host,port)
	ssock.LoginSessionId(sessid)
	return __GetDeploy(ssock,opts)

def SetDeployUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkAlarmDeploySock(host,port)
	ssock.LoginUserPass(username,password)
	return __SetDeploy(ssock,opts)
def SetDeploySessionId(host,port,sessid,opts):
	ssock = xunit.extlib.sdksock.SdkAlarmDeploySock(host,port)
	ssock.LoginSessionId(sessid)
	return __SetDeploy(ssock,opts)

	

def main():
	args = OptionParser()
	args.add_option('-v','--verbose',action='store_true',dest='verbose',help='verbose mode')
	args.add_option('-H','--host',action='store',dest='host',nargs=1,help='specify the host')
	args.add_option('-p','--port',action='store',type='int',dest='port',nargs=1,help='specify the port default is 23')
	args.add_option('-u','--user',action='store',dest='user',nargs=1,help='specify username')
	args.add_option('-P','--pass',action='store',dest='password',nargs=1,help='specify password')
	args.add_option('-s','--session',action='store',dest='session',type='int',nargs=1,help='specify sessionid if has this ,would not login with user password')
	args.add_option('-g','--getcfg',action='store_true',dest='getcfg',default=None,help='specify to get cfg')
	SetArgsInt(args,'scheduleid')
	SetArgsInt(args,'index')
	SetArgsCallback(args,'starttime',StartTimeCallBack)
	SetArgsCallback(args,'endtime',EndTimeCallBack)
	SetArgsCallback(args,'idex',IdexCallBack)
	SetArgsCallback(args,'jdex',JdexCallBack)
	
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)

	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(asctime)s %(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	if options.session :
		if options.getcfg is not None:
			GetDeploySessionId(options.host,options.port,options.session,options)
		else:
			SetDeploySessionId(options.host,options.port,options.session,options)
	else:
		if options.getcfg is not None:
			GetDeployUserPassword(options.host,options.port,options.user,options.password,options)
		else:
			SetDeployUserPassword(options.host,options.port,options.user,options.password,options)
	return

if __name__ == '__main__':
	main()



