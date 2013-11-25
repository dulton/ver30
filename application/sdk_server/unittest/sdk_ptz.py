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

def __GetPreset(ssock):
	presets = ssock.GetPresetInfoCmd()
	for p in presets:
		print('%s'%(repr(p)))
	return presets


def SetPtzUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkPtzSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('session id %s'%(sesid))

	if OptsHasName(opts,'upcmd'):
		sys.stdout.write('Up cmd speed (%d)...'%(opts.upcmd))
		sys.stdout.flush()
		ssock.UpCmd(opts.upcmd)

	if OptsHasName(opts,'downcmd'):
		sys.stdout.write('Down cmd speed (%d)...'%(opts.downcmd))
		sys.stdout.flush()
		ssock.DownCmd(opts.downcmd)


	if OptsHasName(opts,'leftcmd'):
		sys.stdout.write('Left cmd speed (%d)...'%(opts.leftcmd))
		sys.stdout.flush()
		ssock.LeftCmd(opts.leftcmd)

	if OptsHasName(opts,'rightcmd'):
		sys.stdout.write('Right cmd speed (%d)...'%(opts.rightcmd))
		sys.stdout.flush()
		ssock.RightCmd(opts.rightcmd)

	if OptsHasName(opts,'upleftcmd'):
		sys.stdout.write('UpLeft cmd speed (%d)...'%(opts.upleftcmd))
		sys.stdout.flush()
		ssock.UpLeftCmd(opts.upleftcmd)

	if OptsHasName(opts,'downleftcmd'):
		sys.stdout.write('DownLeft cmd speed (%d)...'%(opts.downleftcmd))
		sys.stdout.flush()
		ssock.DownLeftCmd(opts.downleftcmd)


	if OptsHasName(opts,'uprightcmd'):
		sys.stdout.write('UpRight cmd speed (%d)...'%(opts.uprightcmd))
		sys.stdout.flush()
		ssock.UpRightCmd(opts.uprightcmd)

	if OptsHasName(opts,'downrightcmd'):
		sys.stdout.write('DownRight cmd speed (%d)...'%(opts.downrightcmd))
		sys.stdout.flush()
		ssock.DownRightCmd(opts.downrightcmd)

	
	if OptsHasName(opts,'presetname') and OptsHasName(opts,'presetidx'):
		ptzpreset = sdkproto.ptz.PtzPreset()
		ptzpreset.PtzId(1)
		ptzPreset.PresetIdx(opts.presetidx)
		ptzpreset.PresetName(opts.presetname)
		ssock.SetPresetInfoCmd(ptzpreset)

	if OptsHasName(opts,'getpreset'):
		ptzpresets = ssock.GetPresetInfoCmd()
		for p in ptzpresets:
			print('%s'%(str(p)))

	if OptsHasName(opts,'gotopreset'):
		ssock.GotoPresetCmd(opts.gotopreset)

	if OptsHasName(opts,'clearpreset'):
		ssock.ClearPresetCmd(opts.clearpreset)

	if OptsHasName(opts,'setpreset'):
		ssock.SetPresetCmd(opts.setpreset)

	if OptsHasName(opts,'stop'):
		ssock.StopCmd()


	logging.info('\n')

	sys.stdout.write('finished\n')
	return


def GetPresetUserPassword(host,port,username,password)	:
	ssock = xunit.extlib.sdksock.SdkPtzSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('session id %s'%(sesid))
	__GetPreset(ssock)
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

	if OptsHasName(opts,'getpreset'):
		return 
	if OptsHasName(opts,'presetname'):
		if not OptsHasName(opts,'presetidx'):
			Usage(args,3,'please set presetname and presetidx at the same time')
		else:
			return

	if OptsHasName(opts,'presetidx'):
		if not OptsHasName(opts,'presetname'):
			Usage(args,3,'please set presetname and presetidx at the same time')
		else:
			return

	if OptsHasName(opts,'upcmd'):
		return

	if OptsHasName(opts,'downcmd'):
		return
	
	if OptsHasName(opts,'rightcmd'):
		return
	if OptsHasName(opts,'leftcmd'):
		return
	if OptsHasName(opts,'uprightcmd'):
		return

	if OptsHasName(opts,'downrightcmd'):
		return
	
	if OptsHasName(opts,'upleftcmd'):
		return
	if OptsHasName(opts,'downleftcmd'):
		return

	if OptsHasName(opts,'setpreset'):
		return

	if OptsHasName(opts,'gotopreset'):
		return

	if OptsHasName(opts,'clearpreset'):
		return

	if OptsHasName(opts,'stop'):
		return

	Usage(args,3,'please specify the operations')
	return

def main():
	args = OptionParser()
	args.add_option('-v','--verbose',action='store_true',dest='verbose',help='verbose mode')
	args.add_option('-H','--host',action='store',dest='host',nargs=1,help='specify the host')
	args.add_option('-p','--port',action='store',type='int',dest='port',nargs=1,help='specify the port default is 23')
	args.add_option('-u','--user',action='store',dest='user',nargs=1,help='specify username')
	args.add_option('-P','--pass',action='store',dest='password',nargs=1,help='specify password')
	args.add_option('-s','--session',action='store',dest='session',type='int',nargs=1,help='specify sessionid if has this ,would not login with user password')
	args.add_option('-U','--up',action='store',dest='upcmd',type='int',nargs=1,default=None,help='specify ptz up and speed')
	args.add_option('-D','--down',action='store',dest='downcmd',type='int',nargs=1,default=None,help='specify ptz down and speed')
	args.add_option('-L','--left',action='store',dest='leftcmd',type='int',nargs=1,default=None,help='specify ptz left and speed')
	args.add_option('-R','--right',action='store',dest='rightcmd',type='int',nargs=1,default=None,help='specify ptz right and speed')
	args.add_option('--UR','--upright',action='store',dest='uprightcmd',type='int',nargs=1,default=None,help='specify ptz upright and speed')
	args.add_option('--UL','--upleft',action='store',dest='upleftcmd',type='int',nargs=1,default=None,help='specify ptz upleft and speed')
	args.add_option('--DR','--downright',action='store',dest='downrightcmd',type='int',nargs=1,default=None,help='specify ptz downright and speed')
	args.add_option('--DL','--downleft',action='store',dest='downleftcmd',type='int',nargs=1,default=None,help='specify ptz downleft and speed')
	args.add_option('--SP','--setpreset',action='store',dest='setpreset',type='int',nargs=1,default=None,help='set preset (presetidx)')
	args.add_option('--CP','--clearpreset',action='store',dest='clearpreset',type='int',nargs=1,default=None,help='clear preset (presetidx)')
	args.add_option('--GP','--gotopreset',action='store',dest='gotopreset',type='int',nargs=1,default=None,help='goto preset (presetidx)')
	args.add_option('--getpreset',action='store_true',dest='getpreset',default=None,help='get preset all info')
	args.add_option('--presetname',action='store',dest='presetname',default=None,nargs=1,type='string',help='preset name')
	args.add_option('--presetidx',action='store',dest='presetidx',default=None,nargs=1,type='int',help='preset name')
	args.add_option('--stop',action='store_true',dest='stop',default=None,help='set stop command')
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)

	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(asctime)s %(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	if options.session :
		pass
	else:
		if OptsHasName(options,'getpreset'):
			GetPresetUserPassword(options.host,options.port,options.user,options.password)
		else:
			SetPtzUserPassword(options.host,options.port,options.user,options.password,options)
	return

if __name__ == '__main__':
	main()
	



