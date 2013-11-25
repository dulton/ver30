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
import xunit.extlib.sdkproto.ptzpreset as ptzpreset

def OptsHasName(opts,name):
	if not hasattr(opts,name):
		return 0
	if getattr(opts,name) is None:
		return 0

	return 1

def SetArgsStr(args,name):
	args.add_option('--%s'%(name),action='store',nargs=1,type='string',dest='%s'%(name),default=None,help='to set %s'%(name))
	return

def SetArgsInt(args,name):
	args.add_option('--%s'%(name),action='store',nargs=1,type='int',dest='%s'%(name),default=None,help='to set %s'%(name))
	return


def __GetPreset(ssock,opts):
	presets = ssock.GetPreset(1)	
	for p in presets:
		print('%s'%(repr(p)))
	return presets

def __SetPreset(ssock,opts):
	presets = __GetPreset(ssock,opts)
	# now we should find preset
	fp = None
	for p in presets:
		if p.PresetIdx() == opts.presetidx:
			fp = p
			break

	if fp is None:
		fp = ptzpreset.PtzPreset()

	fp.PresetIdx(opts.presetidx)
	fp.PtzId(opts.ptzid)
	fp.PresetName(opts.presetname)
	ssock.SetPreset(fp)

	presets = __GetPreset(ssock,opts)
	return

def GetPresetUserPassword(host,port,user,password,opts):
	ssock = xunit.extlib.sdksock.SdkPtzPresetSock(host,port)
	sesid = ssock.LoginUserPass(user,password)
	logging.info('session id %s'%(sesid))
	__GetPreset(ssock,opts)
	return

def SetPresetUserPassword(host,port,user,password,opts):
	ssock = xunit.extlib.sdksock.SdkPtzPresetSock(host,port)
	sesid = ssock.LoginUserPass(user,password)
	logging.info('session id %s'%(sesid))
	__SetPreset(ssock,opts)
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
	if OptsHasName(opts,'host') == 0:
		Usage(args,3,'please specify host -H')
	if OptsHasName(opts,'port') == 0 or opts.port < 0 or opts.port > (1 << 16):
		Usage(args,3,'please specify port by -p')


	if OptsHasName(opts,'session') :
		logging.info('session %s'%(opts.session))
		if opts.session < 0 or opts.session >= ( 1 << 16):
			Usage(args,3,'session >0 and <= %d'%((1 << 16)))
	else:		
		if not OptsHasName(opts,'user'):
			Usage(args,3,'please specify user by -u')
		if not OptsHasName(opts,'password'):
			Usage(args,3,'please specify password by -P')
			
	if OptsHasName(opts,'getcfg'):
		return

	if not OptsHasName(opts,'presetidx'):
		Usage(args,3,'Please specify presetidx by --presetidx')

	if not OptsHasName(opts,'presetname') and not OptsHasName(opts,'ptzid'):
		Usage(args,3,'Please set name or ptzid by --presetname --ptzid')

	return

def main():
	args = OptionParser()
	args.add_option('-v','--verbose',action='store_true',dest='verbose',help='verbose mode')
	args.add_option('-H','--host',action='store',dest='host',nargs=1,help='specify the host')
	args.add_option('-p','--port',action='store',type='int',dest='port',nargs=1,help='specify the port default is 23')
	args.add_option('-u','--user',action='store',dest='user',nargs=1,help='specify username')
	args.add_option('-P','--pass',action='store',dest='password',nargs=1,help='specify password')
	args.add_option('-s','--session',action='store',dest='session',type='int',nargs=1,help='specify sessionid if has this ,would not login with user password')
	args.add_option('--getcfg',action='store_true',dest='getcfg',default=None,help='get preset all info')
	SetArgsInt(args,'presetidx')
	SetArgsInt(args,'ptzid')
	SetArgsStr(args,'presetname')
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)

	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(asctime)s %(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	if options.session :
		pass
	else:
		if OptsHasName(options,'getcfg'):
			GetPresetUserPassword(options.host,options.port,options.user,options.password,options)
		else:
			SetPresetUserPassword(options.host,options.port,options.user,options.password,options)
	return

if __name__ == '__main__':
	logging.basicConfig(level=logging.INFO,format="%(asctime)s %(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")
	main()
	

