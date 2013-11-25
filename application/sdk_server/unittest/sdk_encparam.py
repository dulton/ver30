import os
import re
import random
import sys
import time
from optparse import OptionParser
import logging
import thread
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__),'..','..','..','..')))
import xunit.extlib.sdksock
import xunit.extlib.sdkproto as sdkproto

def OptsHasName(opts,name):
	if not hasattr(opts,name):
		return 0
	if getattr(opts,name) is None:
		return 0
	return 1

def __GetEncParam(ssock,opts):
	encparam = ssock.GetEncParam()
	print('%s'%(repr(encparam)))
	return encparam


def GetEncParamUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkEncParamSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('[%s:%d]=>[%s:%s]session id %s'%(host,port,username,password,sesid))
	return __GetEncParam(ssock,opts)
	

def __SetEncParam(ssock,opts):
	encparam = __GetEncParam(ssock,opts)
	encparam.VideoId(opts.videoid)
	encparam.EnableStreamNum(opts.enablestreamnum)
	encparam.StreamCombineNo(opts.streamcombineno)
	encparam.Reserv1(opts.reserv1)
	ssock.SetEncParam(encparam)
	encparam = __GetEncParam(ssock,opts)
	return

def SetEncParamUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkEncParamSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('[%s:%d]=>[%s:%s]session id %s'%(host,port,username,password,sesid))
	return __SetEncParam(ssock,opts)
	

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


def VerifyOptions(opts,args):
	if not OptsHasName(opts,'host'):
		Usage(args,3,'please specify host -H')
	if not OptsHasName(opts,'port') or opts.port < 0 or opts.port > (1 << 16):
		Usage(args,3,'please specify port by -p')


	if OptsHasName(opts,'session') :
		logging.info('session %s'%(opts.session))
		if opts.session < 0 or opts.session >= ( 1 << 16):
			Usage(args,3,'session >0 and <= %d'%((1 << 16)))
		return
	
	if not OptsHasName(opts,'user'):
		Usage(args,3,'please specify user by -u')
	if not OptsHasName(opts,'password'):
		Usage(args,3,'please specify password by -P')

	if OptsHasName(opts,'getcfg'):
		return

	if OptsHasName(opts,'enablestreamnum') or OptsHasName(opts,'streamcombineno') :
		return

	if OptsHasName(opts,'videoid') or OptsHasName(opts,'reserv1'):
		return

	Usage(args,3,'please set --enablestreamnum or --streamcombineno or --videoid or --reserv1')
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
	SetArgsInt(args,'videoid')
	SetArgsInt(args,'enablestreamnum')
	SetArgsInt(args,'streamcombineno')
	SetArgsInt(args,'reserv1')
	
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)

	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	if options.session :
		pass
	else:
		if options.getcfg:
			GetEncParamUserPassword(options.host,options.port,options.user,options.password,options)
		else:
			SetEncParamUserPassword(options.host,options.port,options.user,options.password,options)
	return

if __name__ == '__main__':
	main()


