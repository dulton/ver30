#! python


import sys
import os
import re

import time
import logging
sys.path.append(os.path.abspath(os.path.dirname(__file__)))
import realstream
import esparser
from optparse import OptionParser

def GetStream(host,port,streamid=0,frames=0,outf=None,sleeptime=0.0,cont=False):
	ret = 0
	try:
		hostport='%s:%d'%(host,port)
		getframe = 0
		rstream = realstream.RealStream(hostport)
		rstream.Connect()
		streamtype,width,height = rstream.Establish(streamid)
		logging.info('stype %d width %d height %d'%(streamtype,width,height))
		logging.info('frames %d'%(frames))
		if cont:
			maxpacks = 0
			lastfn = 0
		else:
			lastfn = 0
		while frames == 0 or getframe < frames:
	 		buf = rstream.GetFrameMedia()
	 		if outf:
	 			outf.write(buf)
	 			outf.flush()
	 		curfn = esparser.GetFrameNum(buf)
	 		if cont :
	 			if curfn == 0:
	 				if maxpacks == 0 and lastfn != 0:
	 					maxpacks = lastfn
	 				elif maxpacks != 0 :
	 					if maxpacks != lastfn:
	 						raise Exception('maxpacks (%d) != lastfn (%d)'%(maxpacks,lastfn))	
	 			else:
	 				if (curfn) != (lastfn + 1):
	 					raise Exception('curfn %d != (%d + 1)'%(curfn,lastfn))
	 		else:
		 		if curfn != 0:
		 			if curfn != (lastfn + 1):
		 				raise Exception('curfn %d != (%d + 1)'%(curfn,lastfn))
	 		lastfn = curfn
	 		if getframe % 100 == 0:
		 		logging.info('[%d]buf[%d] %s'%(getframe,len(buf),len(buf) > 20 and repr(buf[:20]) or repr(buf)))
		 	if sleeptime != 0.0:
		 		if (getframe % 20) == 0 :
		 			logging.info('sleeptime %f'%(sleeptime))
			 	time.sleep(sleeptime)
			getframe += 1
	except:		
		t ,v,trc = sys.exc_info()
		logging.info('%s (%s) (%s)'%(t,v,trc))
		ret  = -1
	logging.info('exit  frames(%d) ret(%d)'%(getframe,ret))
	return ret

def Usage(opt,ec,msg=None):
	fp = sys.stderr
	if ec == 0:
		fp = sys.stdout
	if msg :
		fp.write('%s\n'%(msg))
	opt.print_help(fp)
	sys.exit(ec)
	
def main():
	args = OptionParser('%s [OPTIONS] host port'%(os.path.basename(__file__)))
	args.add_option('-f','--frames',action='store',dest='frames',nargs=1,default=0,help='specify the port default is 0')
	args.add_option('-s','--sleeptime',action='store',dest='sleeptime',nargs=1,default=0.0,help='specify sleep time for one frame deault is 0.0 no sleep')
	args.add_option('-S','--streamid',action='store',dest='streamid',nargs=1,default=0,help='specify the streamid default is 0')
	args.add_option('-w','--dump',action='store',dest='dumpfile',nargs=1,default=None,help='specify the dump file default none')
	args.add_option('-v','--verbose',action='store_true',dest='verbose',default=False,help='verbose mode')
	args.add_option('-C','--continue',action='store_true',dest='cont',default=False,help='continues mode')
	options , nargs = args.parse_args(sys.argv[1:])
	if len(nargs) < 2:
		Usage(args,3,'please specify  host and ip')
	if options.verbose:
		logging.basicConfig(level=logging.INFO,format="%(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")	
	fh = None
	host = nargs[0]
	port = int(nargs[1])
  	frames = int(options.frames)
  	streamid = int(options.streamid)
  	sleeptime = float(options.sleeptime)
  	cont = options.cont
  	if options.dumpfile:
  		fh = open(options.dumpfile,'w+b')

	ret = GetStream(host,port,streamid,frames,fh,sleeptime,cont)	

  	if fh:
  		close(fh)
 	sys.exit(ret)

if __name__ == '__main__':
	main()
