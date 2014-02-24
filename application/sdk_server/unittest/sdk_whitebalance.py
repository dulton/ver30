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

def __GetWhiteBalance(ssock,opts):
	wb = ssock.GetWhiteBalance()
	return wb


def GetWhiteBalanceUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkWhiteBalanceSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('session id %s'%(sesid))
	wb = __GetWhiteBalance(ssock,opts)
	print('%s'%(repr(wb)))
	return 


def GetWhiteBalanceSessionId(host,port,sesid,opts):
	ssock = xunit.extlib.sdksock.SdkWhiteBalanceSock(host,port)
	sesid = ssock.LoginSessionId(sesid)
	logging.info('session id %s'%(sesid))
	wb = __GetWhiteBalance(ssock,opts)
	print('%s'%(repr(wb)))
	return

def __SetWhiteBalance(ssock,opts):

	wb = __GetWhiteBalance(ssock,opts)
	print('Before Get :')
	print('%s'%(repr(wb)))

	wb.Mode(opts.mode)
	wb.RGrain(opts.rgrain)
	wb.BGrain(opts.bgrain)
	wb.Reserv1(opts.reserv1)

	print('Set to :')
	print('%s'%(repr(wb)))

	ssock.SetWhiteBalance(wb)

	wb = __GetWhiteBalance(ssock,opts)
	print('After Get :')
	print('%s'%(repr(wb)))
	return

def SetWhiteBalanceUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkWhiteBalanceSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('session id %s'%(sesid))
	__SetWhiteBalance(ssock,opts)
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
	if not OptsHasName(opts,'host'):
		Usage(args,3,'please specify host -H')
	if not OptsHasName(opts,'port') or opts.port is None or opts.port < 0 or opts.port > (1 << 16):
		Usage(args,3,'please specify port by -p')


	if OptsHasName(opts,'session') :
		logging.info('session %s'%(opts.session))
		if opts.session < 0 or opts.session >= ( 1 << 16):
			Usage(args,3,'session >0 and <= %d'%((1 << 16)))
		return
	else:
		if not OptsHasName(opts,'user'):
			Usage(args,3,'please specify user by -u or --user')	
		if not OptsHasName(opts,'password'):
			Usage(args,3,'please specify password by -P or --password')
		

	if OptsHasName(opts,'getcfg'):
		return

	if OptsHasName(opts,'mode'):
		return

	if OptsHasName(opts,'rgrain'):
		return

	if OptsHasName(opts,'bgrain'):
		return

	if OptsHasName(opts,'reserv1'):
		return

	Usage(args,3,'please set -g or other whitebalance setting')
	return

def SetArgsInt(args,name,defval=None):
	if defval is not None:
		args.add_option('--%s'%(name),action='store',nargs=1,type='int',dest='%s'%(name),default=defval,help='to set %s default %s'%(name,repr(defval)))
	else:
		args.add_option('--%s'%(name),action='store',nargs=1,type='int',dest='%s'%(name),default=defval,help='to set %s default None'%(name))


def main():
	args = OptionParser()
	args.add_option('-v','--verbose',action='count',dest='verbose',help='verbose mode if -vvv debug most')
	args.add_option('-H','--host',action='store',dest='host',nargs=1,help='specify the host')
	args.add_option('-p','--port',action='store',type='int',dest='port',nargs=1,help='specify the port default is 23')
	args.add_option('-u','--user',action='store',dest='user',nargs=1,help='specify username')
	args.add_option('-P','--pass',action='store',dest='password',nargs=1,help='specify password')
	args.add_option('-s','--session',action='store',dest='session',type='int',nargs=1,help='specify sessionid if has this ,would not login with user password')
	args.add_option('-g','--getcfg',action='store_true',dest='getcfg',default=False,help='get video cfg')
	SetArgsInt(args,'mode')
	SetArgsInt(args,'rgrain')
	SetArgsInt(args,'bgrain')
	SetArgsInt(args,'reserv1')
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)

	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	if options.session :
		if options.getcfg :
			GetWhiteBalanceSessionId(options.host,options.port,options.session,options)
		else:
			pass
	else:
		if options.getcfg:
			GetWhiteBalanceUserPassword(options.host,options.port,options.user,options.password,options)
		else:
			SetWhiteBalanceUserPassword(options.host,options.port,options.user,options.password,options)
	return

if __name__ == '__main__':
	main()
	



