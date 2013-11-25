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

def OptsHasName(opts,name):
	if not hasattr(opts,name):
		return 0
	if getattr(opts,name) is None:
		return 0

	return 1

def __GetIpInfo(ssock,opts):
	addrs = ssock.GetIpInfo()
	i = 0
	for a in addrs:
		print('[%d]=============\n%s'%(i,str(a)))
		i += 1
	return addrs

def __SetIpInfo(ssock,opts):
	print('Before Set ========================')
	addrs = __GetIpInfo(ssock,opts)
	netinfo=None
	for info in addrs:
		if OptsHasName(opts,'ifname'):
			if info.IfName() == opts.ifname:
				netinfo = info
				break
		else:
			netinfo = info
			break
	if netinfo is None:
		raise Exception('%s ifname not find'%(opts.ifname))

	netinfo.IpAddr(opts.ipaddr)
	netinfo.SubMask(opts.submask)
	netinfo.Gateway(opts.gateway)
	netinfo.Dns(opts.dns)
	netinfo.HwAddr(opts.hwaddr)
	netinfo.Dhcp(opts.dhcp)

	print('Set Netinfo==============\n%s====================\n'%(str(netinfo)))
	ssock.SetInfoTimeout(netinfo,3.0)
	print('After Set =======================')
	addrs  = __GetIpInfo(ssock,opts)
	return


def GetIpInfoUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkIpInfoSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('session id %s'%(sesid))
	__GetIpInfo(ssock,opts)
	return


def GetIpInfoSessionId(host,port,sesid,opts):
	ssock = xunit.extlib.sdksock.SdkIpInfoSock(host,port)
	sesid = ssock.LoginSessionId(sesid)
	logging.info('session id %s'%(sesid))
	__GetIpInfo(ssock,opts)
	return


def SetIpAddrUserName(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkIpInfoSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('session id %s'%(sesid))
	__SetIpInfo(ssock,opts)
	return

def SetIpAddrSessionId(host,port,sesid,opts):
	ssock = xunit.extlib.sdksock.SdkIpInfoSock(host,port)
	sesid = ssock.LoginSessionId(sesid)
	logging.info('session id %s'%(sesid))
	__SetIpInfo(ssock,opts)
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

	if OptsHasName(opts,'getcfg'):
		return
	if OptsHasName(opts,'ipaddr'):
		return
	if OptsHasName(opts,'submask'):
		return
	if OptsHasName(opts,'gateway'):
		return
	if OptsHasName(opts,'hwaddr'):
		return
	if OptsHasName(opts,'dhcp'):
		return

	Usage(args,3,'please specify the getcfg or name')
	return

def SetArgsInt(args,name,defval=None):
	if defval is not None:
		args.add_option('--%s'%(name),action='store',nargs=1,type='int',dest='%s'%(name),default=defval,help='to set %s default %s'%(name,repr(defval)))
	else:
		args.add_option('--%s'%(name),action='store',nargs=1,type='int',dest='%s'%(name),default=defval,help='to set %s default None'%(name))

def SetArgsFloat(args,name,defval=None):
	if defval is not None:
		args.add_option('--%s'%(name),action='store',nargs=1,type='float',dest='%s'%(name),default=defval,help='to set %s default %s'%(name,repr(defval)))
	else:
		args.add_option('--%s'%(name),action='store',nargs=1,type='float',dest='%s'%(name),default=defval,help='to set %s default None'%(name))


def SetArgsStr(args,name,defval=None):
	if defval is not None:
		args.add_option('--%s'%(name),action='store',nargs=1,type='string',dest='%s'%(name),default=defval,help='to set %s default %s'%(name,repr(defval)))
	else:
		args.add_option('--%s'%(name),action='store',nargs=1,type='string',dest='%s'%(name),default=defval,help='to set %s default None'%(name))


def main():
	args = OptionParser()
	args.add_option('-v','--verbose',action='count',dest='verbose',help='verbose mode if -vvv debug most')
	args.add_option('-H','--host',action='store',dest='host',nargs=1,help='specify the host')
	args.add_option('-p','--port',action='store',type='int',dest='port',nargs=1,help='specify the port default is 23')
	args.add_option('-u','--user',action='store',dest='user',nargs=1,help='specify username')
	args.add_option('-P','--pass',action='store',dest='password',nargs=1,help='specify password')
	args.add_option('-s','--session',action='store',dest='session',type='int',nargs=1,help='specify sessionid if has this ,would not login with user password')
	args.add_option('-g','--getcfg',action='store_true',dest='getcfg',default=None,help='to get the ipinfo')
	SetArgsStr(args,'ipaddr')
	SetArgsStr(args,'hwaddr')
	SetArgsStr(args,'dns')
	SetArgsStr(args,'submask')
	SetArgsStr(args,'gateway')
	SetArgsInt(args,'dhcp')
	SetArgsStr(args,'ifname')
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)

	
	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	logging.info('nargs %s'%(repr(nargs)))
	if OptsHasName(options,'getcfg') :
		if OptsHasName(options,'session'):
			GetIpInfoSessionId(options.host,options.port,options.session,options)
		else:
			GetIpInfoUserPassword(options.host,options.port,options.user,options.password,options)
	else:
		if  OptsHasName(options,'session'):
			SetIpAddrSessionId(options.host,options.port,options.session,options)
		else:
			SetIpAddrUserName(options.host,options.port,options.user,options.password,options)

	
	return

if __name__ == '__main__':
	main()
	
