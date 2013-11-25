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



def GetVideoCfgUserPassword(host,port,username,password):
	ssock = xunit.extlib.sdksock.SdkVideoCfgSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('session id %s'%(sesid))
	vcfgs = ssock.GetVideoCfg()
	for v in vcfgs:
		print('%s'%(str(v)))
	return


def GetVideoCfgSessionId(host,port,sesid):
	ssock = xunit.extlib.sdksock.SdkVideoCfgSock(host,port)
	sesid = ssock.LoginSessionId(sesid)
	logging.info('session id %s'%(sesid))
	vcfgs = ssock.GetVideoCfg()
	for v in vcfgs:
		print('%s'%(str(v)))
	return

def SetVideoCfgUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkVideoCfgSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('session id %s'%(sesid))
	vcfgs = ssock.GetVideoCfg()
	print('first get --------------\n')
	for v in vcfgs:
		print('%s'%(str(v)))
	print('first get +++++++++++++++\n')
	assert(opts.streamid is not None)

	fv = None
	for v in vcfgs:
		if v.Flag() == opts.streamid:
			fv = v
			break

	if fv is None:
		raise Exception('not find (%d) streamid'%(opts.streamid))

	fv.StreamType(opts.streamtype)
	fv.Compression(opts.compression)
	fv.PicWidth(opts.picwidth)
	fv.PicHeight(opts.picheight)
	fv.BitrateCtrl(opts.bitratectrl)
	fv.Quality(opts.quality)
	fv.Fps(opts.fps)
	fv.BitrateAverage(opts.bitrateaverage)
	fv.BitrateUp(opts.bitrateup)
	fv.BitrateDown(opts.bitratedown)
	fv.Gop(opts.gop)
	fv.Rotate(opts.rotate)

	print('set video cfg\n//////////////////\n%s\\\\\\\\\\\\\\\\\\\\\\\\\\\\\n'%(str(fv)))
	ssock.SetVideoCfg(fv)


	vcfgs = ssock.GetVideoCfg()
	print('second get --------------\n')
	for v in vcfgs:
		print('%s'%(str(v)))
	print('second get +++++++++++++++\n')

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

	if hasattr(opts,'getcfg') and opts.getcfg:
		return

	if not hasattr(opts,'streamid') or opts.streamid is None:
		Usage(args,3,'set will first -S or --streamid to set for streamid')

	if OptsHasName(opts,'streamtype'):
		return

	if hasattr(opts,'compression') and opts.compression is not None:
		return

	if hasattr(opts,'picwidth') and opts.picwidth is not None:
		return

	if hasattr(opts,'picheight') and opts.picheight is not None:
		return

	if hasattr(opts,'bitratectrl') and opts.bitratectrl is not None:
		return

	if hasattr(opts,'quality') and opts.quality is not None:
		return
	if hasattr(opts,'fps') and opts.fps is not None:
		return
	if hasattr(opts,'bitrateaverage') and opts.bitrateaverage is not None:
		return

	if hasattr(opts,'bitrateup') and opts.bitrateup is not None:
		return

	if hasattr(opts,'bitratedown') and opts.bitratedown is not None:
		return

	if hasattr(opts,'gop') and opts.gop is not None:
		return

	if hasattr(opts,'rotate') and opts.rotate is not None:
		return

	if hasattr(opts,'flag') and opts.flag is not None:
		return

	Usage(args,3,'please set -g or other video setting')
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
	args.add_option('-g','--get',action='store_true',dest='getcfg',default=False,help='get video cfg')
	SetArgsInt(args,'streamid')
	SetArgsInt(args,'streamtype')
	SetArgsInt(args,'compression')
	SetArgsInt(args,'picwidth')
	SetArgsInt(args,'picheight')
	SetArgsInt(args,'bitratectrl')
	SetArgsInt(args,'quality')
	SetArgsInt(args,'fps')
	SetArgsInt(args,'bitrateaverage')
	SetArgsInt(args,'bitrateup')
	SetArgsInt(args,'bitratedown')
	SetArgsInt(args,'gop')
	SetArgsInt(args,'rotate')
	SetArgsInt(args,'flag')
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)

	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	logging.info('nargs %s'%(repr(nargs)))
	if options.session :
		if options.getcfg :
			GetVideoCfgSessionId(options.host,options.port,options.session)
		else:
			pass
	else:
		if options.getcfg:
			GetVideoCfgUserPassword(options.host,options.port,options.user,options.password)	
		else:
			SetVideoCfgUserPassword(options.host,options.port,options.user,options.password,options)
	return

if __name__ == '__main__':
	main()
	


