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



def SetShowCfgUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkShowCfgSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('session id %s'%(sesid))
	showcfgs = ssock.GetShowCfg()
	setcfg = None

	for s in showcfgs:
		if s.Flag() == opts.flag:
			setcfg = s
			break
	if setcfg is None:
		raise Exception('could not find %d'%(opts.flag))

	print('orig cfg --------------\n%s+++++++++++++++++++++\n'%(repr(setcfg)))
	
	setcfg.TmEnable(opts.tmenable)
	setcfg.TmLanguage(opts.tmlanguage)
	setcfg.TmDisplayX(opts.tmdisplayx)
	setcfg.TmDisplayY(opts.tmdisplayy)
	setcfg.TmDateStyle(opts.tmdatestyle)
	setcfg.TmTimeStyle(opts.tmtimestyle)
	setcfg.TmFontColor(opts.tmfontcolor)
	setcfg.TmFontSize(opts.tmfontsize)
	setcfg.TmFontBold(opts.tmfontbold)
	setcfg.TmFontRotate(opts.tmfontrotate)
	setcfg.TmFontItalic(opts.tmfontitalic)
	setcfg.TmFontOutline(opts.tmfontoutline)
	setcfg.ChEnable(opts.chenable)
	setcfg.ChDisplayX(opts.chdisplayx)
	setcfg.ChDisplayY(opts.chdisplayy)
	setcfg.ChFontColor(opts.chfontcolor)
	setcfg.ChFontSize(opts.chfontsize)
	setcfg.ChFontBold(opts.chfontbold)
	setcfg.ChFontRotate(opts.chfontrotate)
	setcfg.ChFontItalic(opts.chfontitalic)
	setcfg.ChFontOutline(opts.chfontoutline)
	setcfg.ChChannelName(opts.chchannelname)

	print('set cfg --------------\n%s+++++++++++++++++++++\n'%(repr(setcfg)))

	ssock.SetShowCfg(setcfg)
	showcfgs = ssock.GetShowCfg()
	for s in showcfgs:
		print('%s'%(str(s)))

	return
	
def GetShowCfgUserPassword(host,port,username,password):
	ssock = xunit.extlib.sdksock.SdkShowCfgSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('session id %s'%(sesid))
	showcfgs = ssock.GetShowCfg()
	for s in showcfgs:
		print('%s'%(str(s)))
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

	if OptsHasName(opts,'tmenable') or OptsHasName(opts,'tmlanguage') or OptsHasName(opts,'tmdisplayx') or \
		OptsHasName(opts,'tmdisplayy') or OptsHasName(opts,'tmdatestyle') or \
		OptsHasName(opts,'tmtimestyle') or 	OptsHasName(opts,'tmfontsize') or \
		OptsHasName(opts,'tmfontcolor') or OptsHasName(opts,'tmfontbold') or \
		OptsHasName(opts,'tmfontrotate') or OptsHasName(opts,'tmfontitalic') or \
		OptsHasName(opts,'tmfontoutline') or OptsHasName(opts,'chenable') or \
		OptsHasName(opts,'chdisplayx') or OptsHasName(opts,'chdisplayy') or \
		OptsHasName(opts,'chfontsize') or OptsHasName(opts,'chfontcolor') or OptsHasName(opts,'chfontbold') or \
		OptsHasName(opts,'chfontrotate') or OptsHasName(opts,'chfontitalic') or OptsHasName(opts,'chfontoutline')  or \
		OptsHasName(opts,'chchannelname') :
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
	SetArgs(args,'flag')
	SetArgs(args,'tmenable')
	SetArgs(args,'tmlanguage')
	SetArgs(args,'tmdisplayx')
	SetArgs(args,'tmdisplayy')
	SetArgs(args,'tmdatestyle')
	SetArgs(args,'tmtimestyle')
	SetArgs(args,'tmfontcolor')
	SetArgs(args,'tmfontsize')
	SetArgs(args,'tmfontbold')
	SetArgs(args,'tmfontrotate')
	SetArgs(args,'tmfontitalic')
	SetArgs(args,'tmfontoutline')
	SetArgs(args,'chenable')
	SetArgs(args,'chdisplayx')
	SetArgs(args,'chdisplayy')
	SetArgs(args,'chfontcolor')
	SetArgs(args,'chfontsize')
	SetArgs(args,'chfontbold')
	SetArgs(args,'chfontrotate')
	SetArgs(args,'chfontitalic')
	SetArgs(args,'chfontoutline')
	SetArgs(args,'chchannelname')
	
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)

	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	if options.session :
		pass
	else:
		if options.getcfg:
			GetShowCfgUserPassword(options.host,options.port,options.user,options.password)
		else:
			SetShowCfgUserPassword(options.host,options.port,options.user,options.password,options)
	return

if __name__ == '__main__':
	main()
	




