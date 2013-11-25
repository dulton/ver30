#! python


import sys
import xunit.config
import xunit.utils.exptel
import xunit.case
import xunit.suite
import xunit.result
import xunit.logger
from optparse import OptionParser
import logging
import xunit.extlib.nfsmap
import os
import re
import random
import time
import xunit.extlib.cfgexptel
import subprocess

sys.path.append(os.path.abspath(os.path.dirname(__file__)))
import realstream
import esparser

CharacterUse='abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789'

class TcpLogin(xunit.extlib.cfgexptel.CfgExpTel):
	def KillTcpStream(self):
		while True:
			ret,getstr = self.Execute('pgrep tcp_stream_server')
			pids = getstr.split('\r\n')
			if len(pids) == 0:
				break
			killpid = -1
			vpat = re.compile('[0-9]+')
			for i in xrange(0,len(pids)):
				if vpat.match(pids[i]):
					killpid = int(pids[i])
					break
			if killpid == -1:
				break
			cmd = 'kill -2 %d'%(killpid)
			self.Execute(cmd)
			time.sleep(0.2)
		return

class TcpStreamUnit(xunit.case.XUnitCase):
	@classmethod
	def XUnitsetUpClass(cls):
		copied = 0
		tcpunit = TcpLogin()
		utcfg = xunit.config.XUnitConfig()
		try:
			tcpunit.Login()
			mntdir = utcfg.GetValue('.nfsmap','mntdir','/mnt/nfs')
			tmpdir = utcfg.GetValue('.nfsmap','tmpdir','/tmp')
			#logging.info('curdir %s\n'%(os.path.abspath(os.path.dirname(__file__))))
			mapdir = xunit.extlib.nfsmap.GetMapDir(os.path.abspath(os.path.dirname(__file__)))
			platform = utcfg.GetValue('build','platform','ambarella_a5s_sdk_v3.3')
			topdir = utcfg.GetValue('build','topdir',os.path.abspath(os.path.join(os.path.abspath(os.path.dirname(__file__)),'..','..','..')))
			sourcedir = os.path.abspath(os.path.join(os.path.abspath(os.path.dirname(__file__)),'..','source'))
			
			#logging.info('sourcedir %s type %s\n'%(sourcedir,type(sourcedir)))
			cutlen = len(topdir)
			partdir = sourcedir[cutlen:]
			while partdir[0] == '/' :
				partdir = partdir[1:]
			#logging.info('partdir %s topdir %s\n'%(partdir,topdir))
			releasedir = os.path.join(topdir,'output',platform,partdir)
			releasebin = os.path.join(releasedir,'tcp_stream_server')
			# now we should get the
			releasemapbin = xunit.extlib.nfsmap.MapNfsDir(mntdir,mapdir,releasebin)			
			#logging.info('releasemapbin %s'%(releasemapbin))
			cmd = 'cp -f %s %s/tcp_stream_server'%(releasemapbin,tmpdir)
			tcpunit.Execute(cmd)
			copied = 1
			# now we should run this			
			tcpunit.Logout()
			del tcpunit
			tcpunit = None
		except :
			e = xunit.utils.exception.XUnitException('login exception')
			if copied and tcpunit:
				mntdir = utcfg.GetValue('.nfsmap','mntdir','/mnt/nfs')
				tmpdir = utcfg.GetValue('.nfsmap','tmpdir','/tmp')
				cmd = 'rm -f %s/tcp_stream_server'%(tmpdir)
				tcpunit.Execute(cmd)
			logging.error('exception %s'%(e))
			logging.info('\n')
			tcpunit.Logout()
			del tcpunit
			tcpunit = None
			raise e			
		return

	@classmethod
	def XUnittearDownClass(cls):
		tcpunit = TcpLogin()
		try:
			tcpunit.Login()
			utcfg = xunit.config.XUnitConfig()
			tmpdir = utcfg.GetValue('.nfsmap','tmpdir','/tmp')
			cmd = 'rm -f   %s/tcp_stream_server'%(tmpdir)
			tcpunit.Execute(cmd)			
		except:
			e = xunit.utils.exception.XUnitException('login exception')
			logging.info('exception %s'%(e))
			raise e			
		finally:
			tcpunit.Logout()
			del tcpunit
			tcpunit = None
		return

	def __ResetTcpStream(self):
		anologin = TcpLogin()
		anologin.Login()
		anologin.KillTcpStream()
		anologin.Logout()
		del anologin
		anologin = None
		return
	def __ClearResource(self):
		if  self.__exptel:
			self.__ResetTcpStream()
			self.__exptel.Logout()
			del self.__exptel
		self.__exptel = None
	def XUnitsetUp(self):
		if hasattr(self,'__exptel') and self.__exptel:
			self.__ClearResource()
		self.__exptel = TcpLogin()
		self.__exptel.Login()
		return
	def XUnittearDown(self):
		self.__ClearResource()
		return


	def test_tcp_connect_ok(self):
		loops = 0
		rstream = None
		random.seed(time.time())
		utcfg = xunit.config.XUnitConfig()
		while True:
			try:
				rport = random.randint(3000,60000)
				tmpdir = utcfg.GetValue('.nfsmap','tmpdir','/tmp')
				self.__exptel.WriteLine('%s/tcp_stream_server %d'%(tmpdir,rport))
				time.sleep(1.0)
		 		host = utcfg.GetValue('.telnet','host','')
		 		hostport = '%s:%d'%(host,rport)
		 		#logging.info('hostport %s'%(hostport))
		 		rstream = realstream.RealStream(hostport)
		 		rstream.Connect()
		 		streamtype,width,height = rstream.Establish()
		 		#logging.info('stype %d width %d height %d'%(streamtype,width,height))
		 		buf = rstream.GetFrameMedia()
		 		#logging.info('buf[%d] %s'%(len(buf),repr(buf[:20])))
		 		# now to test the format for the file
		 		if ord(buf[0]) != 0:
		 			raise xunit.utils.exception.XUnitException('not valid buf %s'%(repr(buf[:10])))
		 		if ord(buf[1]) != 0:
		 			raise xunit.utils.exception.XUnitException('not valid buf %s'%(repr(buf[:10])))
		 		if ord(buf[2]) != 0:
		 			raise xunit.utils.exception.XUnitException('not valid buf %s'%(repr(buf[:10])))
		 		if ord(buf[3]) != 0x1 or ord(buf[4]) != 0x9 or ord(buf[5]) != 0x10:
		 			raise xunit.utils.exception.XUnitException('not valid buf %s'%(repr(buf[:10])))
		 		break
		 	except:
		 		loops += 1
		 		if loops >= 3:
			 		raise xunit.utils.exception.XUnitException('can not connect ok')
			 	e = xunit.utils.exception.XUnitException('')
			 	logging.error('[%d] error %s'%(loops,e))
				if rstream :
			 		rstream.CloseSocket()
			 		del rstream
			 	rstream = None
			 	self.__ResetTcpStream()
		
	 	if rstream :
	 		rstream.CloseSocket()
	 		del rstream
	 	rstream = None
 		return

 	def test_tcp_not_send_streamid(self):
		loops = 0
		rstream = None
		random.seed(time.time())
		utcfg = xunit.config.XUnitConfig()
		while True:
			try:
				rport = random.randint(3000,60000)
				tmpdir = utcfg.GetValue('.nfsmap','tmpdir','/tmp')
				self.__exptel.WriteLine('%s/tcp_stream_server %d'%(tmpdir,rport))
				time.sleep(1.0)
		 		host = utcfg.GetValue('.telnet','host','')
		 		hostport = '%s:%d'%(host,rport)
		 		#logging.info('hostport %s'%(hostport))
		 		rstream = realstream.RealStream(hostport)
		 		rstream.Connect()
		 	except:
		 		loops += 1
		 		if loops >= 3:
			 		raise xunit.utils.exception.XUnitException('can not connect ok')
			 	e = xunit.utils.exception.XUnitException('')
			 	logging.error('[%d] error %s'%(loops,e))
				if rstream :
			 		rstream.CloseSocket()
			 		del rstream
			 	rstream = None
			 	self.__ResetTcpStream()
			 	continue
			 # we do not send any streamid ,so we should get read error
			ok = 1
			try:
				buf = rstream.GetFrameMedia()
			except:
				ok = 0
			self.assertEqual(ok,0)
			break
	 	if rstream :
	 		rstream.CloseSocket()
	 		del rstream
	 	rstream = None
 		return

 	def test_get_frame_sequence_or_i_frame(self):
		loops = 0
		rstream = None
		random.seed(time.time())
		utcfg = xunit.config.XUnitConfig()
		rbuf = None
		while True:
			try:
				rport = random.randint(3000,60000)
				tmpdir = utcfg.GetValue('.nfsmap','tmpdir','/tmp')
				self.__exptel.WriteLine('%s/tcp_stream_server %d'%(tmpdir,rport))
				time.sleep(1.0)
		 		host = utcfg.GetValue('.telnet','host','')
		 		hostport = '%s:%d'%(host,rport)
		 		#logging.info('hostport %s'%(hostport))
		 		rstream = realstream.RealStream(hostport)
		 		rstream.Connect()
		 	except:
		 		loops += 1
		 		if loops >= 3:
			 		raise xunit.utils.exception.XUnitException('can not connect ok')
			 	e = xunit.utils.exception.XUnitException('')
			 	logging.error('[%d] error %s'%(loops,e))
				if rstream :
			 		rstream.CloseSocket()
			 		del rstream
			 	rstream = None
			 	self.__ResetTcpStream()
			 	continue

			st,width,height = rstream.Establish(0)
			getframes = 0
			lastframenum = 0
			while getframes < 200:
				try:
					buf = rstream.GetFrameMedia()
					curframenum = esparser.GetFrameNum(buf)
					if curframenum != 0:
						self.assertEqual(lastframenum + 1,curframenum)
					lastframenum = curframenum
					# we should sleep ,this will give the tcp_stream_server 
					# discard packet
					sys.stderr.write('.')
					if (getframes % 20) == 0 and getframes:
						sys.stderr.write('\n')
					sys.stderr.flush()
					time.sleep(0.2)
					rbuf,ps1 = self.__exptel.ReadImmediate()
					getframes += 1
				except:
					e = xunit.utils.exception.XUnitException('can not receive frame on %d'%(getframes))
					logging.info(e)
					raise e
			break
		if rstream :
	 		rstream.CloseSocket()
	 		del rstream
	 	rstream = None
	 	self.__ResetTcpStream()
		return			 

 	def test_continouos_packets(self):
		loops = 0
		rstream = None
		random.seed(time.time())
		utcfg = xunit.config.XUnitConfig()
		while True:
			try:
				rport = random.randint(3000,60000)
				tmpdir = utcfg.GetValue('.nfsmap','tmpdir','/tmp')
				self.__exptel.WriteLine('%s/tcp_stream_server %d'%(tmpdir,rport))
				time.sleep(0.5)
		 		host = utcfg.GetValue('.telnet','host','')
		 		hostport = '%s:%d'%(host,rport)
		 		#logging.info('hostport %s'%(hostport))
		 		rstream = realstream.RealStream(hostport)
		 		rstream.Connect()
		 	except:
		 		loops += 1
		 		if loops >= 3:
			 		raise xunit.utils.exception.XUnitException('can not connect ok')
			 	e = xunit.utils.exception.XUnitException('')
			 	logging.error('[%d] error %s'%(loops,e))
				if rstream :
			 		rstream.CloseSocket()
			 		del rstream
			 	rstream = None
			 	self.__ResetTcpStream()
			 	continue

			st,width,height = rstream.Establish(0)
			getframes = 0
			mostpacks = 0
			lastframenum = 0
			while getframes < 1000:
				try:
					buf = rstream.GetFrameMedia()
					curframenum = esparser.GetFrameNum(buf)
					if curframenum != 0:
						if (lastframenum + 1)!=curframenum:
							raise xunit.utils.exception.XUnitException('frame[%d] (%d + 1) != %d'%(getframes,lastframenum,curframenum))
					else:
						if mostpacks == 0 and lastframenum != 0:
							mostpacks = lastframenum
						else :
							if mostpacks != lastframenum:
								raise xunit.utils.exception.XUnitException('frame[%d] (%d ) != %d'%(getframes,lastframenum,mostpacks))
							
					lastframenum = curframenum
					
					if (getframes % 10) == 0:
						sys.stderr.write('.')
						if (getframes % 100) == 0 and getframes:
							sys.stderr.write('\n')
							rbuf,ps1 = self.__exptel.ReadImmediate()
							
					sys.stderr.flush()
					# we should sleep ,this will give the tcp_stream_server 
					# discard packet
					getframes += 1
				except:
					e = xunit.utils.exception.XUnitException('can not receive frame on %d'%(getframes))
					logging.info(e)
					raise e
			break
		if rstream :
	 		rstream.CloseSocket()
	 		del rstream
	 	rstream = None
	 	self.__ResetTcpStream()
		return			 

 		
 		
 		
 	def test_timeout_packets(self):
		loops = 0
		rstream = None
		random.seed(time.time())
		utcfg = xunit.config.XUnitConfig()
		while True:
			try:
				rport = random.randint(3000,60000)
				tmpdir = utcfg.GetValue('.nfsmap','tmpdir','/tmp')
				self.__exptel.WriteLine('%s/tcp_stream_server %d'%(tmpdir,rport))
				time.sleep(1.0)
		 		host = utcfg.GetValue('.telnet','host','')
		 		hostport = '%s:%d'%(host,rport)
		 		#logging.info('hostport %s'%(hostport))
		 		rstream = realstream.RealStream(hostport)
		 		rstream.Connect()
		 	except:
		 		loops += 1
		 		if loops >= 3:
			 		raise xunit.utils.exception.XUnitException('can not connect ok')
			 	e = xunit.utils.exception.XUnitException('')
			 	logging.error('[%d] error %s'%(loops,e))
				if rstream :
			 		rstream.CloseSocket()
			 		del rstream
			 	rstream = None
			 	self.__ResetTcpStream()
			 	continue

			st,width,height = rstream.Establish(0)
			getframes = 0
			mostpacks = 0
			lastframenum = 0
			while getframes < 6:
				rbuf,ps1 = self.__exptel.ReadImmediate()
				time.sleep(1.0)
				getframes += 1
			ok = 1
			try:
				while getframes < 100:
					buf = rstream.GetFrameMedia() 
					getframes += 1
			except:
				ok =0
			# we have timeout ,so it must failed
			self.assertEqual(0,ok)
			break
		if rstream :
	 		rstream.CloseSocket()
	 		del rstream
	 	rstream = None
	 	self.__ResetTcpStream()
		return

	def __TerminateClients(self,mclients,mosttimeout):
		retcode = 0
		curretcode = 0
		mosttime = mosttimeout * 5
		runtimes = 0
		while len(mclients)>0:
			cproc = mclients[0]
			if cproc.poll():
				time.sleep(0.2)
				runtimes += 1
				if runtimes > mosttime:
					cproc.terminate()
				continue
			runtimes = 0
			curretcode = cproc.returncode
			if curretcode != 0:
				retcode = curretcode
			del mclients[0]
			cproc = None
		return retcode,mclients

	def __WaitClients(self,mclients,timeout):
		retcode=0
		curretcode = 0
		ctime = time.time()
		ltime = ctime
		etime = ctime + timeout
		cnt = 0
		try:
			while len(mclients) > 0:
				#logging.info('ctime %d etime %d\n'%(ctime,etime))
				if timeout != 0 and etime < ctime:
					break
				if ltime  < ctime:
					cnt += 1
					sys.stderr.write('%d'%(len(mclients)))
					if (cnt % 10) == 0:
						sys.stderr.write('\n')
					sys.stderr.flush()
					ltime = ctime
				for i in xrange(0,len(mclients)):
					cproc = mclients[i]
					polret = cproc.poll()
					#logging.info('cproc %s (%s)'%(repr(cproc),repr(polret)))
					if   polret is not None:
						curretcode = cproc.returncode
						#logging.info('cproc %s return (%s)'%(repr(cproc),repr(curretcode)))
						if curretcode is None:
							break
						if curretcode != 0:
							retcode = curretcode
						del mclients[i]
						break
				time.sleep(.5)
				self.__exptel.ReadImmediate()
				ctime = time.time()
		except:
			retcode = 255
			e = xunit.utils.exception.XUnitException('could not wait')
			logging.info(e)
			raise e
		#logging.info('retcode %d mclients(%s)\n'%(retcode,mclients))
		return retcode,mclients

	def __StartCmd(self,cmd):
		cproc = subprocess.Popen(cmd,shell=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
		#cproc = subprocess.Popen(cmd,shell=True,stdout=sys.stdout,stderr=sys.stderr)
		return cproc



	def __TestServerOk(self,host,port):
		utcfg = xunit.config.XUnitConfig()
		topdir = utcfg.GetValue('build','topdir','')
		filedir = os.path.dirname(os.path.abspath(__file__))
		exportdir = os.path.abspath(os.path.join(topdir,'..'))
		cmd = 'PYTHONPATH=$PYTHONPATH:%s python %s/reals.py -v -f 20  %s %d'%(exportdir,filedir,host,port)
		#logging.info('cmd (%s)'%(cmd))
		cproc = self.__StartCmd(cmd)
		ctime = time.time()
		etime = ctime + 30
		cnt = 0
		ltime = ctime
		while ctime < etime:
			if ltime < ctime:
				cnt += 1
				sys.stderr.write('T')
				if (cnt % 10) == 0:
					sys.stderr.write('\n')
				sys.stderr.flush()
			retcode = cproc.poll()
			if retcode is None:
				time.sleep(0.5)
				ctime = time.time()
				continue			
			if retcode != 0:
				raise xunit.utils.exception.XUnitException('run (%s) not ok (%s)'%(cmd,retcode))
			del cproc
			cproc = None
			return 0
		del cproc
		cproc = None		
		raise xunit.utils.exception.XUnitException('run (%s) timeout'%(cmd))
			
		

	def test_aamulti_clients(self):
		mclients = []
		utcfg = xunit.config.XUnitConfig()
		topdir = utcfg.GetValue('build','topdir','')
		filedir = os.path.dirname(os.path.abspath(__file__))
		host = utcfg.GetValue('.telnet','host','')
		tmpdir = utcfg.GetValue('.nfsmap','tmpdir','/tmp')
		exportdir = os.path.abspath(os.path.join(topdir,'..'))
		loops = 0
		while True:
			try:
				rport = random.randint(3000,60000)
				# we should redirect output for it will give the debug info
				self.__exptel.WriteLine('%s/tcp_stream_server %d >/dev/null'%(tmpdir,rport))
				# wait for a while ,as running ok
				time.sleep(0.5)
				self.__TestServerOk(host,rport)
				break
			except:
				loops += 1
				if loops >= 3:
					e= xunit.utils.exception.XUnitException('could not server ok')
					logging.error(e)
					raise e
				# ok this is ok
				self.__ResetTcpStream()
				
		
		for i in xrange(0,5):
			streamid = 0
			if (i%2):
				streamid = 1
			cmd = 'PYTHONPATH=$PYTHONPATH:%s python %s/reals.py -v -f 300 -S %d %s %d'%(exportdir,filedir,streamid,host,rport)
			cproc = self.__StartCmd(cmd)
			#logging.info('cmd[%d] (%s) %s\n'%(i,cmd,repr(cproc)))
			mclients.append(cproc)
			time.sleep(1.0)

		i = 0
		retcode = 0
		curretcode , mclients = self.__WaitClients(mclients,0)
		if curretcode != 0:
			retcode = curretcode

	
		self.assertEqual(len(mclients),0)

		self.assertEqual(retcode , 0)

		# sleep for a while ,when the server reset to ok
		time.sleep(5.0)
		return


			 


def SplitVariables(v):
	#logging.info('v %s'%(v))
	p = '\[([^]]+)\]\.([^=]+)=(.*)'
	vpat = re.compile(p)
	m = vpat.match(v)
	if not m:
		raise exception.XUnitException('(%s) not match (%s)'%(v,p))
	sec = m.group(1)
	opt = m.group(2)
	val = m.group(3)
	return sec,opt,val

def SetOuterVariables(utcfg,variables=[]):
	# first to parse the variable
	for va in variables:
		s,o,v = SplitVariables(va)
		utcfg.SetValue(s,o,v,1)
	return


def Usage(opt,ec,msg=None):
	fp = sys.stderr
	if ec == 0:
		fp = sys.stdout
	if msg :
		fp.write('%s\n'%(msg))
	opt.print_help(fp)
	sys.exit(ec)


def Parse_Callback(option, opt_str, value, parser):
	#print 'option %s opt_str %s value %s parser %s'%(repr(option), repr(opt_str), repr(value), repr(parser))
	if opt_str == '-D' or opt_str == '--variable':
		if not hasattr(parser.values,'variables'):
			parser.values.variables = []
		if value :
			parser.values.variables.append(value)
	elif opt_str == '-V' or opt_str == '--debug':
		if not hasattr(parser.values,'variables'):
			parser.values.variables = []
		parser.values.variables.append('[global].xmllevel=5')
		
	else:
		Usage(parser,3,'unknown option (%s)'%(option))



def maintest():
	sbase = xunit.suite.XUnitSuiteBase()
	# now for the name of current case
	mn = '__main__'
	cn = 'TcpStreamUnit'
	sbase.LoadCase(mn +'.'+cn)
	
	_res = xunit.result.XUnitResultBase(1)

	for s in sbase:
		s.run(_res)
		if _res.shouldStop:
			break
	if _res.Fails() != 0 or _res.UnexpectSuccs() !=0:
		sys.exit(3)
	sys.exit(0)


def main():
	args = OptionParser()
	args.add_option('-v','--verbose',action='count',dest='verbose',help='verbose mode if -vvv debug most')
	args.add_option('-f','--failfast',action="store_true",dest="failfast",help="failfast mode")
	args.add_option('-D','--variable',action="callback",callback=Parse_Callback,type="string",nargs=1,help='specify a variable format is [section].option=value')
	args.add_option('-V','--debug',action="callback",callback=Parse_Callback,help='debug mode ')
	args.add_option('-H','--host',action='store',dest='host',nargs=1,help='specify the host')
	args.add_option('-p','--port',action='store',dest='port',nargs=1,help='specify the port default is 23')
	args.add_option('-u','--user',action='store',dest='user',nargs=1,help='specify username')
	args.add_option('-P','--pass',action='store',dest='password',nargs=1,help='specify password')
	args.add_option('-l','--login',action='store',dest='loginnote',nargs=1,help='login note default is (login:)')
	args.add_option('-e','--passnote',action='store',dest='passwordnote',nargs=1,help='password note default is (assword:)')
	args.add_option('-c','--cmdnote',action='store',dest='cmdnote',nargs=1,help='cmd note default is (# )')
	args.add_option('-t','--timeout',action='store',dest='timeout',nargs=1,help='set timeout value default is 5')
	options ,nargs = args.parse_args(sys.argv[1:])
	
	utcfg = xunit.config.XUnitConfig()
	utcfg.SetValue('build','topdir',os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..','..')))
	if len (nargs) >= 1:
		utcfg.LoadFile(nargs[0])
	verb = 0
	ff = False
	if options.verbose:
		verb = options.verbose
	if options.failfast:
		ff = True
	if options.host:
		utcfg.SetValue('.telnet','host',options.host,1)
	if options.port:
		utcfg.SetValue('.telnet','port',options.port,1)
	if options.user:
		utcfg.SetValue('.telnet','username',options.user,1)
	if options.password:
		utcfg.SetValue('.telnet','password',options.password,1)

	if options.loginnote:
		utcfg.SetValue('.telnet','loginnote',options.loginnote,1)
	if options.passwordnote:
		utcfg.SetValue('.telnet','passwordnote',options.passwordnote,1)
	if options.cmdnote:
		utcfg.SetValue('.telnet','cmdnote',options.cmdnote,1)

	if verb :
		logging.basicConfig(level=logging.INFO,format="%(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")		
		utcfg.SetValue('global','debug.mode','y',1)
		if verb >= 3:
			utcfg.SetValue('__main__','xmllevel',5)
	else:
		logging.basicConfig(level=logging.WARNING,format="%(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")
		utcfg.SetValue('global','debug.mode','n',1)

	if ff:
		utcfg.SetValue('global','failfast','y',1)
	else:
		utcfg.SetValue('global','failfast','n',1)

	if hasattr(options,'xmllog'):
		utcfg.SetValue('global','xmllog',options.xmllog,1)
	# now for the name of current case
	maintest()

if __name__ == '__main__':
	main()
