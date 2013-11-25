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


def GetSysCfgUserPassword(host,port,username,password):
	ssock = xunit.extlib.sdksock.SdkSysCfgSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('session id %s'%(sesid))
	syscfg = ssock.GetSysCfg()
	print('%s'%(str(syscfg)))
	return


def GetSysCfgSessionId(host,port,sesid):
	ssock = xunit.extlib.sdksock.SdkSysCfgSock(host,port)
	sesid = ssock.LoginSessionId(sesid)
	logging.info('session id %s'%(sesid))
	syscfg = ssock.GetSysCfg()
	print('%s'%(str(syscfg)))
	return


def __SetSysCfg(ssock,opts):
	syscfg = ssock.GetSysCfg()
	print('%s'%(str(syscfg)))

	if hasattr(opts,'devicename'):
		syscfg.DeviceName(opts.devicename)

	if hasattr(opts,'deviceid'):
		syscfg.DeviceId(opts.deviceid)

	if hasattr(opts,'devicemodel'):
		syscfg.DeviceModel(opts.devicemodel)

	if hasattr(opts,'devicemanufactor'):
		syscfg.DeviceManufactor(opts.devicemanufactor)

	if hasattr(opts,'devicesn'):
		syscfg.DeviceSN(opts.devicesn)

	if hasattr(opts,'devicefwver'):
		syscfg.DeviceFWVer(opts.devicefwver)

	if hasattr(opts,'devicehwver'):
		syscfg.DeviceHWVer(opts.devicehwver)

	print('set cfg\n+++++++++++++++++++++++++\n%s\n+++++++++++++++++++++++++\n'%(str(syscfg)))
	ssock.SetSysCfg(syscfg)

	syscfg = ssock.GetSysCfg()
	print('reset syscfg\n=======================\n%s\n==========================\n'%(str(syscfg)))
	return


def SetSysCfgUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkSysCfgSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('session id %s'%(sesid))
	__SetSysCfg(ssock,opts)
	return

def SetSysCfgSessionId(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkSysCfgSock(host,port)
	sesid = ssock.LoginSessionId(sesid)
	logging.info('session id %s'%(sesid))
	__SetSysCfg(ssock,opts)
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

	logging.error('getcfg %s'%(opts.getcfg))
	if hasattr(opts,'getcfg') and opts.getcfg :
		return

	if hasattr(opts,'devicename') and opts.devicename is not None:
		return

	if hasattr(opts,'devicemodel') and opts.devicemodel is not None:
		return

	if hasattr(opts,'deviceid') and opts.deviceid is not None:
		return

	if hasattr(opts,'devicemanufactor') and opts.devicemanufactor is not None:
		return

	if hasattr(opts,'devicesn') and opts.devicesn is not None:
		return

	if hasattr(opts,'devicefwver') and opts.devicefwver is not None:
		return

	if hasattr(opts,'devicehwver') and opts.devicehwver is not None:
		return

	Usage(args,3,'please set device if you do not set get')
	return

def main():
	args = OptionParser()
	args.add_option('-v','--verbose',action='count',dest='verbose',help='verbose mode if -vvv debug most')
	args.add_option('-H','--host',action='store',dest='host',nargs=1,help='specify the host')
	args.add_option('-p','--port',action='store',type='int',dest='port',nargs=1,help='specify the port default is 23')
	args.add_option('-u','--user',action='store',dest='user',nargs=1,help='specify username')
	args.add_option('-P','--pass',action='store',dest='password',nargs=1,help='specify password')
	args.add_option('-s','--session',action='store',dest='session',type='int',nargs=1,help='specify sessionid if has this ,would not login with user password')
	args.add_option('-g','--get',action='store_true',dest='getcfg',default=False,help='get video cfg')
	args.add_option('-D','--devicename',action='store',dest='devicename',default=None,help='set device name')
	args.add_option('-m','--devicemodel',action='store',dest='devicemodel',default=None,help='set device model')
	args.add_option('-I','--deviceid',action='store',type='int',dest='deviceid',default=None,help='set device id')
	args.add_option('-M','--devicemanufactor',action='store',dest='devicemanufactor',default=None,help='set device manufactor')
	args.add_option('-S','--devicesn',action='store',dest='devicesn',default=None,help='set device sn')
	args.add_option('-f','--devicefwver',action='store',dest='devicefwver',default=None,help='set device fw version')
	args.add_option('-w','--devicehwver',action='store',dest='devicehwver',default=None,help='set device hw version')
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)

	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	logging.info('nargs %s'%(repr(nargs)))
	if options.session :
		if options.getcfg:
			GetSysCfgSessionId(options.host,options.port,options.session)
		else:
			SetSysCfgSessionId(options.host,options.port,options.session,options)
	else:
		if options.getcfg:
			GetSysCfgUserPassword(options.host,options.port,options.user,options.password)
		else:
			SetSysCfgUserPassword(options.host,options.port,options.user,options.password,options)
	return

if __name__ == '__main__':
	main()

