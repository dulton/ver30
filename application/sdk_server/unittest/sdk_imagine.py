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



def SetImagineUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkImagineSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('session id %s'%(sesid))
	img = ssock.GetImagine()

	print('orig cfg --------------\n%s+++++++++++++++++++++\n'%(repr(img)))
	

	print('set cfg --------------\n%s+++++++++++++++++++++\n'%(repr(img)))
	img.VideoId(opts.videoid)
	img.Brightness(opts.brightness)
	img.Contrast(opts.contrast)
	img.Saturation(opts.saturation)
	img.Hue(opts.hue)
	img.Sharpness(opts.sharpness)
	img.ExposureMode(opts.exposuremode)
	img.ExposureValueMin(opts.exposurevaluemin)
	img.ExposureValueMax(opts.exposurevaluemax)
	img.GainMax(opts.gainmax)
	print('set before ------------\n%s+++++++++++++++++++++++\n'%(repr(img)))
	ssock.SetImagine(img)
	img = ssock.GetImagine()
	print('set after ------------\n%s+++++++++++++++++++++++\n'%(repr(img)))

	return
	
def GetImagineUserPassword(host,port,username,password):
	ssock = xunit.extlib.sdksock.SdkImagineSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('session id %s'%(sesid))
	img = ssock.GetImagine()

	print('%s'%(repr(img)))
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

	if OptsHasName(opts,'videoid') or OptsHasName(opts,'brightness') or \
		OptsHasName(opts,'contrast') or OptsHasName(opts,'saturation') or \
		OptsHasName(opts,'hue') or 	OptsHasName(opts,'sharpness') or \
		OptsHasName(opts,'exposuremode') or OptsHasName(opts,'exposurevaluemin') or \
		OptsHasName(opts,'exposurevaluemax') or OptsHasName(opts,'gainmax'):
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
	SetArgs(args,'brightness')
	SetArgs(args,'contrast')
	SetArgs(args,'saturation')
	SetArgs(args,'hue')
	SetArgs(args,'sharpness')
	SetArgs(args,'exposuremode')
	SetArgs(args,'exposurevaluemin')
	SetArgs(args,'exposurevaluemax')
	SetArgs(args,'gainmax')
	
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)

	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	if options.session :
		pass
	else:
		if options.getcfg:
			GetImagineUserPassword(options.host,options.port,options.user,options.password)
		else:
			SetImagineUserPassword(options.host,options.port,options.user,options.password,options)
	return

if __name__ == '__main__':
	main()
	





