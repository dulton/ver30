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
import xunit.extlib.sdkproto.netport as CLSNetPort

def OptsHasName(opts,name):
	if not hasattr(opts,name):
		return 0
	if getattr(opts,name) is None:
		return 0

	return 1


def __GetNetPort(ssock):
	netport = ssock.GetNetworkPort()
	print('%s'%(repr(netport)))
	return netport
	
def GetNetportUserPassword(host,port,username,password):
	ssock = xunit.extlib.sdksock.SdkNetworkPortSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	return __GetNetPort(ssock)

def __SetNetport(ssock,opts):
	netport =__GetNetPort(ssock)

	netport.HttpPort(opts.httpport)
	netport.SdkPort(opts.sdkport)
	netport.RtspPort(opts.rtspport)
	print('Set ===============\n%s\n====================\n'%(repr(netport)))
	ssock.SetNetworkPort(netport)

	netport = __GetNetPort(ssock)
	
	return

def SetNetportUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkNetworkPortSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	return __SetNetport(ssock,opts)




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

	if OptsHasName(opts,'httpport'):
		return

	if OptsHasName(opts,'rtspport'):
		return

	if OptsHasName(opts,'sdkport'):
		return


	Usage(args,3,'please specify Options')

	return 

def SetArgsInt(args,name):
	args.add_option('--%s'%(name),action='store',nargs=1,type='int',dest='%s'%(name),default=None,help='to set %s'%(name))
	return


def main():
	args = OptionParser()
	args.add_option('-v','--verbose',action='store_true',dest='verbose',help='verbose mode')
	args.add_option('-H','--host',action='store',dest='host',nargs=1,help='specify the host')
	args.add_option('-p','--port',action='store',type='int',dest='port',nargs=1,help='specify the port default is 23')
	args.add_option('-u','--user',action='store',dest='user',nargs=1,help='specify username')
	args.add_option('-P','--pass',action='store',dest='password',nargs=1,help='specify password')
	args.add_option('-s','--session',action='store',dest='session',type='int',nargs=1,help='specify sessionid if has this ,would not login with user password')
	args.add_option('-g','--getcfg',action='store_true',dest='getcfg',default=None,help='to get userinfo')
	SetArgsInt(args,'httpport')
	SetArgsInt(args,'rtspport')
	SetArgsInt(args,'sdkport')
	
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)

	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	if options.session :
		pass
	else:
		if OptsHasName(options,'getcfg'):
			GetNetportUserPassword(options.host,options.port,options.user,options.password)
		else:
			SetNetportUserPassword(options.host,options.port,options.user,options.password,options)
	return

if __name__ == '__main__':
	main()

