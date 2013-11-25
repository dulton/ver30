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

def __GetAdvImagine(ssock):
	advimagine =  ssock.GetAdvImagine()
	print('=============================================\n%s========================================\n'%(repr(advimagine)))
	return advimagine

def __SetAdvImagine(ssock,opts):
	advimagine = __GetAdvImagine(ssock)
	print('orig ===================\n%s'%(repr(advimagine)))
	advimagine.VideoId(opts.videoid)
	advimagine.MeteringMode(opts.meteringmode)
	advimagine.BackLightCompFlag(opts.backlightcompflag)
	advimagine.DcIrisFlag(opts.dcirisflag)
	advimagine.LocalExposure(opts.localexposure)
	advimagine.MctfStrength(opts.mctfstrength)
	advimagine.DcIrisDuty(opts.dcirisduty)
	advimagine.AeTargetRatio(opts.aetargetratio)
	print('set ===================\n%s'%(repr(advimagine)))
	ssock.SetAdvImagine(advimagine)
	__GetAdvImagine(ssock)
	return 

def SetAdvImagineUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkAdvImagineSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('session id %s'%(sesid))
	__SetAdvImagine(ssock,opts)
	return
	
def GetAdvImagineUserPassword(host,port,username,password):
	ssock = xunit.extlib.sdksock.SdkAdvImagineSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('session id %s'%(sesid))
	__GetAdvImagine(ssock)
	return

def Usage(opt,ec,msg=None):
	fp = sys.stderr
	if ec == 0:
		fp = sys.stdout
	if msg :
		fp.write('%s\n'%(msg))
	opt.print_help(fp)
	sys.exit(ec)

def OptsHasName(opts,name):
	if not hasattr(opts,name):
		return 0
	if getattr(opts,name) is None:
		return 0

	return 1


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

	if OptsHasName(opts,'videoid') or OptsHasName(opts,'meteringmode') or \
		OptsHasName(opts,'backlightcompflag') or OptsHasName(opts,'dcirisflag') or \
		OptsHasName(opts,'localexposure') or 	OptsHasName(opts,'mctfstrength') or \
		OptsHasName(opts,'dcirisduty') or OptsHasName(opts,'aetargetratio') :
		return 

	Usage(args,3,'please specify one name')

	return 


def SetArgs(args,name):
	args.add_option('--%s'%(name),action='store',nargs=1,type='int',dest='%s'%(name),default=None,help='to set %s'%(name))

def main():
	args = OptionParser()
	args.add_option('-v','--verbose',action='store_true',dest='verbose',help='verbose mode')
	args.add_option('-H','--host',action='store',dest='host',nargs=1,help='specify the host')
	args.add_option('-p','--port',action='store',type='int',dest='port',nargs=1,help='specify the port default is 23')
	args.add_option('-u','--user',action='store',dest='user',nargs=1,help='specify username')
	args.add_option('-P','--pass',action='store',dest='password',nargs=1,help='specify password')
	args.add_option('-s','--session',action='store',dest='session',type='int',nargs=1,help='specify sessionid if has this ,would not login with user password')
	args.add_option('-g','--getcfg',action='store_true',dest='getcfg',default=None,help='to get showcfg')
	SetArgs(args,'videoid')
	SetArgs(args,'meteringmode')
	SetArgs(args,'backlightcompflag')
	SetArgs(args,'dcirisflag')
	SetArgs(args,'localexposure')
	SetArgs(args,'mctfstrength')
	SetArgs(args,'dcirisduty')
	SetArgs(args,'aetargetratio')
	
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)

	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	if options.session :
		pass
	else:
		if options.getcfg:
			GetAdvImagineUserPassword(options.host,options.port,options.user,options.password)
		else:
			SetAdvImagineUserPassword(options.host,options.port,options.user,options.password,options)
	return

if __name__ == '__main__':
	main()
	





