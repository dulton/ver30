#! python

import os
import sys
import logging
import subprocess
import socket
import thread
import traceback
from optparse import OptionParser
sys.path.append(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..','..','..')))
import xunit.config
import xunit.suite
import xunit.result
import xunit.extlib.sdksock
import xunit.extlib.sdkproto as sdkproto
from xunit.utils import exptel
from xunit.utils import exception
import xunit.extlib.cfgexptel as cfgexptel
import xunit.extlib.nfsmap as nfsmap
import re
import time
import random


class RemoteConnectError(exception.XUnitException):
	pass

class ProcessStillRunningError(exception.XUnitException):
	pass

class ProcessRunningError(exception.XUnitException):
	pass

class UserNotInsertError(exception.XUnitException):
	pass

	
class InvalidParameterError(exception.XUnitException):
	pass


class AudioThreadCtrl:
	def __init__(self):
		self.running = 1
		self.rcvexited = 0
		self.sndexited = 0
		self.audiorcvd = 0
		self.audiosnd = 0
		self.audiotime = 3.0
		self.rcvres = 2
		self.sndres = 2
		return

	def __del__(self):
		self.running = 1
		self.rcvexited = 0
		self.sndexited = 0
		self.audiorcvd = 0
		self.audiosnd = 0
		self.audiotime = 3.0
		self.rcvres = 2
		self.sndres = 2
		return
		

class SdkLoginUnit(xunit.case.XUnitCase):
	def SdkStreamLoginIn(self):
		utcfg = xunit.config.XUnitConfig()
		host = utcfg.GetValue('.sdkserver','host','')
		port = utcfg.GetValue('.sdkserver','port','30000')
		username = utcfg.GetValue('.sdkserver','username',None)
		password = utcfg.GetValue('.sdkserver','password',None)
		port = int(port)
		username = str(username)
		password = str(password)
		host = str(host)
		ssock = xunit.extlib.sdksock.SdkStreamSock(host,port)
		sesid = ssock.LoginUserPass(username,password)
		return ssock,sesid

	def SdkNetworkPortLoginIn(self):
		utcfg = xunit.config.XUnitConfig()
		host = utcfg.GetValue('.sdkserver','host','')
		port = utcfg.GetValue('.sdkserver','port','30000')
		username = utcfg.GetValue('.sdkserver','username',None)
		password = utcfg.GetValue('.sdkserver','password',None)
		port = int(port)
		username = str(username)
		password = str(password)
		host = str(host)		
		ssock = xunit.extlib.sdksock.SdkNetworkPortSock(host,port)
		sesid = ssock.LoginUserPass(username,password)
		return ssock,sesid

	def SdkUserInfoLoginIn(self):
		utcfg = xunit.config.XUnitConfig()
		host = utcfg.GetValue('.sdkserver','host','')
		port = utcfg.GetValue('.sdkserver','port','30000')
		username = utcfg.GetValue('.sdkserver','username',None)
		password = utcfg.GetValue('.sdkserver','password',None)
		port = int(port)
		username = str(username)
		password = str(password)
		host = str(host)		
		ssock = xunit.extlib.sdksock.SdkUserInfoSock(host,port)
		#logging.info('username %s password %s'%(username,password))
		sesid = ssock.LoginUserPass(username,password)
		return ssock,sesid

	def SdkVideoCfgLogin(self):
		utcfg = xunit.config.XUnitConfig()
		host = utcfg.GetValue('.sdkserver','host','')
		port = utcfg.GetValue('.sdkserver','port','30000')
		username = utcfg.GetValue('.sdkserver','username',None)
		password = utcfg.GetValue('.sdkserver','password',None)
		port = int(port)
		username = str(username)
		password = str(password)
		host = str(host)		
		ssock = xunit.extlib.sdksock.SdkVideoCfgSock(host,port)
		#logging.info('username %s password %s'%(username,password))
		sesid = ssock.LoginUserPass(username,password)
		return ssock,sesid

	def SdkAudioDualLogin(self):
		utcfg = xunit.config.XUnitConfig()
		host = utcfg.GetValue('.sdkserver','host','')
		port = utcfg.GetValue('.sdkserver','port','30000')
		username = utcfg.GetValue('.sdkserver','username',None)
		password = utcfg.GetValue('.sdkserver','password',None)
		port = int(port)
		username = str(username)
		password = str(password)
		host = str(host)		
		ssock = xunit.extlib.sdksock.SdkAudioDualSock(host,port)
		#logging.info('username %s password %s'%(username,password))
		sesid = ssock.LoginUserPass(username,password)
		return ssock,sesid

	def SdkIpInfoLogin(self):
		utcfg = xunit.config.XUnitConfig()
		host = utcfg.GetValue('.sdkserver','host','')
		port = utcfg.GetValue('.sdkserver','port','30000')
		username = utcfg.GetValue('.sdkserver','username',None)
		password = utcfg.GetValue('.sdkserver','password',None)
		port = int(port)
		username = str(username)
		password = str(password)
		host = str(host)		
		ssock = xunit.extlib.sdksock.SdkIpInfoSock(host,port)
		#logging.info('username %s password %s'%(username,password))
		sesid = ssock.LoginUserPass(username,password)
		return ssock,sesid

	def SdkTimeLogin(self):
		utcfg = xunit.config.XUnitConfig()
		host = utcfg.GetValue('.sdkserver','host','')
		port = utcfg.GetValue('.sdkserver','port','30000')
		username = utcfg.GetValue('.sdkserver','username',None)
		password = utcfg.GetValue('.sdkserver','password',None)
		port = int(port)
		username = str(username)
		password = str(password)
		host = str(host)		
		ssock = xunit.extlib.sdksock.SdkTimeSock(host,port)
		sesid = ssock.LoginUserPass(username,password)
		return ssock,sesid

	def SdkSessionHeartBeatLogin(self,sesid):
		utcfg = xunit.config.XUnitConfig()
		host = utcfg.GetValue('.sdkserver','host','')
		port = utcfg.GetValue('.sdkserver','port','30000')
		port = int(port)
		host = str(host)		
		ssock = xunit.extlib.sdksock.SdkSock(host,port)
		sesid = ssock.LoginHeartBeat(sesid)
		return ssock,sesid
		

	def FindPids(self,flts,unflts):
		fltcmd = ''
		for f in flts:
			fltcmd += ' | grep -e \'%s\' '%(f)
		
		cmd = 'ps -ef '
		if len(flts) > 0:
			cmd += fltcmd

		unfltcmd =''
		for uf in unflts:
			unfltcmd += ' | grep -v \'%s\' '%(uf)
		if len(unflts) > 0:
			cmd += unfltcmd
		cmd += ' | grep -v grep'
		cmd += ' | awk \'{print $2}\''
		pssub = subprocess.Popen(cmd,shell=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
		allpids = pssub.stdout.readlines()
		pids = []
		for p in allpids:
			pids.append(int(p))
		return pids

	def KillPids(self,sig,pids):
		for p in pids:
			os.kill(p,sig)
		return

	def CallProcessStart(self,cmd,stdout=False,stderr=False):
		_cmd = cmd
		if not stdout:
			_cmd += ' >/dev/null'
		if not stderr:
			_cmd += ' 2>/dev/null'
			
		cmds = [ '/bin/sh','-c',_cmd]
		#logging.info('run command %s'%(repr(cmds)))
		pid = os.spawnvpe(os.P_NOWAIT,'/bin/sh',cmds,os.environ)
		return  pid


	def WaitPidsReturn(self,pids):
		removepid = 0
		retval = 0
		for p in pids:
			retpids = os.waitpid(p,os.P_NOWAIT)
			if retpids[0] != 0:
				removepid = retpids[0]
				retval = retpids[1]
				break
		if removepid != 0:
			pids.remove(removepid)
		return pids,removepid,retval

	def GetIpAddress(self):
		s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		s.connect(("www.sina.com.cn",80))
		ipaddr = s.getsockname()[0]
		return ipaddr

	def IsMountDir(self):
		ipaddr = self.GetIpAddress()
		cmdtel = None
		cmdtel = cfgexptel.CfgExpTel()
		cmdtel.Login()
		cmdstr = 'mount | grep %s '%(ipaddr)
		matched,result = cmdtel.Execute(cmdstr)
		cmdtel.Logout()
		fidx = result.find(ipaddr)
		if fidx < 0:
			matched = 0
		else:
			matched = 1
		return matched
	

	def Mountdir(self):
		mounted = self.IsMountDir()
		ipaddr = self.GetIpAddress()
		utcfg = xunit.config.XUnitConfig()
		tmpdir = utcfg.GetValue('.telnet','tmpdir','/tmp')
		mntdir = utcfg.GetValue('.telnet','mountdir','/mnt/nfs')
		buildtop = utcfg.GetValue('build','topdir','.')
		if mounted == 0:
			# now we should 
			cmdstr = 'mkdir -p %s'%(mntdir)
			cmdtel = cfgexptel.CfgExpTel()
			cmdtel.Login()
			cmdtel.Execute(cmdstr)
			# now we should get the mount dir
			localnfsdir = nfsmap.GetMapDir(buildtop)
			cmdstr = 'mount -t nfs %s:%s %s -o nolock'%(ipaddr,localnfsdir,mntdir)
			cmdtel.Execute(cmdstr)
			mounted = self.IsMountDir()
			self.assertEqual(mounted ,1)
			cmdtel.Logout()
		return

	def TelnetExecCmd(self,cmd,cmdtime=0):
		utcfg = xunit.config.XUnitConfig()
		host = utcfg.GetValue('.telnet','host','')
		port = utcfg.GetValue('.telnet','port','23')
		user = utcfg.GetValue('.telnet','username',None)
		password = utcfg.GetValue('.telnet','password',None)
		loginnote = utcfg.GetValue('.telnet','loginnote','login:')
		passwordnote = utcfg.GetValue('.telnet','passwordnote','assword:')
		cmdnote = utcfg.GetValue('.telnet','cmdnote','# ')
		timeout = utcfg.GetValue('.telnet','timeout','5')
		port = int(port)
		timeout = float(timeout)
		__tel = None
		try:
			__tel = exptel.XUnitTelnet(host,port,user,password,None,timeout,loginnote,passwordnote,cmdnote)
			__tel.Execute(cmd,cmdtime)
			del __tel
			__tel = None
		except:
			raise RemoteConnectError('can not connect [%s:%s]=>[user:%s;password:%s]'%(host,port,user,password))
		finally:
			if __tel:
				del __tel
			__tel = None

	def CopyNfsFile(self,fromfile,todir):
		#first to map 
		utcfg = xunit.config.XUnitConfig()
		tmpdir = utcfg.GetValue('.telnet','tmpdir','/tmp')
		mntdir = utcfg.GetValue('.telnet','mountdir','/mnt/nfs')
		buildtop = utcfg.GetValue('build','topdir','.')
		cmdtel = cfgexptel.CfgExpTel()
		cmdtel.Login()
		# to map the file and to set for 
		mapdir = nfsmap.GetMapDir(buildtop)
		mapfile = nfsmap.MapNfsDir(mntdir,mapdir,fromfile)
		basemapfile = os.path.basename(mapfile)
		#logging.info('copy %s(map:%s) => %s '%(fromfile,mapfile,'%s/%s'%(todir,basemapfile)))
		cmdstr = 'cp -f %s %s/%s'%(mapfile,todir,basemapfile)
		cmdtel.Execute(cmdstr)
		cmdtel.Logout()
		return 

	def RemoteKill(self,exename):
		cmdtel = cfgexptel.CfgExpTel()
		cmdtel.Login()
		# to map the file and to set for 
		while 1:
			cmdstr = 'pkill -2 %s'%(exename)
			cmdtel.Execute(cmdstr)
			time.sleep(1)
			cmdstr = 'ps -ef | grep %s | grep -v grep'%(exename)
			matched,result = cmdtel.Execute(cmdstr)
			fidx = result.find(exename)
			if fidx < 0 :
				break
		cmdtel.Logout()
		return
		

	def UnMountdir(self):
		utcfg = xunit.config.XUnitConfig()
		mntdir = utcfg.GetValue('.telnet','mountdir','/mnt/nfs')
		cmdstr = 'umount %s'%(mntdir)
		cmdtel = cfgexptel.CfgExpTel()
		cmdtel.Login()
		cmdtel.Execute(cmdstr)
		cmdtel.Logout()	
		return


class InitLoginUnit(SdkLoginUnit):
	def test_a1checkok(self):
		stime = time.time()
		etime = stime + 80
		ctime = stime
		ssock = None
		ltime = ctime
		while ctime < etime:
			try:
				ssock,sesid = self.SdkStreamLoginIn()
				break				
			except:
				if ssock :
					ssock.CloseSocket()
					del ssock
				ssock = None
			if int(ltime) != int(ctime):
				sys.stdout.write('%d'%(int(etime - ctime)))
				sys.stdout.flush()
				ltime = ctime
			time.sleep(1)
			ctime = time.time()
		self.assertTrue(ssock is not None)
		ssock.CloseSocket()
		del ssock
		ssock = None		
		return

	def __ChangeHost(self,host):
		cips = host.split('.')
		self.assertEqual(len(cips),4)
		cints = []
		for c in cips:
			cints.append(int(c))
		newhost = '%d.%d.%d.%d'%(cints[0],cints[1],cints[2],cints[3]-1)
		return newhost

	def __ChangeHostTest(self,orighost,newhost):
		utcfg = xunit.config.XUnitConfig()
		ssock,sesid = self.SdkIpInfoLogin()
		ipinfos = ssock.GetIpInfo()
		ipinfo = ipinfos[0]
		newhost = self.__ChangeHost(orighost)

		ipinfo.IpAddr(newhost)
		ok = 1
		try:
			ssock.SetInfoTimeout(ipinfo,3.0)
		except xunit.extlib.sdksock.SdkSockRecvTimeoutError as e:
			ok = 0
		self.assertEqual(ok,0)		
		ssock.CloseSocket()
		ssock = None
		# now we should
		utcfg.SetValue('.sdkserver','host','%s'%(newhost),1)
		sys.stdout.write('N')
		sys.stdout.flush()
		time.sleep(1.0)

		# now for the orig set
		ssock,sesid = self.SdkStreamLoginIn()
		ssock.StartStream(['0'])
		ssock.CloseSocket()
		ssock = None
		sys.stdout.write('S')
		sys.stdout.flush()

		# now we change back into the
		ssock,sesid = self.SdkIpInfoLogin()
		ipinfos  = ssock.GetIpInfo()
		ipinfo = ipinfos[0]
		ipinfo.IpAddr(orighost)
		ok = 1
		try:
			ssock.SetInfoTimeout(ipinfo,3.0)
		except xunit.extlib.sdksock.SdkSockRecvTimeoutError as e:
			ok = 0
		self.assertEqual(ok,0)		
		ssock.CloseSocket()
		ssock = None
		# now we should
		utcfg.SetValue('.sdkserver','host','%s'%(orighost),1)
		sys.stdout.write('O')
		return
		

	def notest_a2ChangeIp(self):
		# now to change the ip address
		utcfg = xunit.config.XUnitConfig()
		ssock,sesid = self.SdkIpInfoLogin()
		ipinfos = ssock.GetIpInfo()
		ipinfo = ipinfos[0]
		orighost = ipinfo.IpAddr()
		newhost = self.__ChangeHost(orighost)

		for i in xrange(10):
			self.__ChangeHostTest(orighost,newhost)
		
		return
		


class RebootUnit(SdkLoginUnit):
	def __RebootRemote(self):
		# first to make sure that the remote to reboot
		utcfg = xunit.config.XUnitConfig()
		host = utcfg.GetValue('.telnet','host','')
		port = utcfg.GetValue('.telnet','port','23')
		user = utcfg.GetValue('.telnet','username',None)
		password = utcfg.GetValue('.telnet','password',None)
		loginnote = utcfg.GetValue('.telnet','loginnote','login:')
		passwordnote = utcfg.GetValue('.telnet','passwordnote','assword:')
		cmdnote = utcfg.GetValue('.telnet','cmdnote','# ')
		timeout = utcfg.GetValue('.telnet','timeout','5')
		port = int(port)
		timeout = float(timeout)
		__tel = None
		try:
			__tel = exptel.XUnitTelnet(host,port,user,password,None,timeout,loginnote,passwordnote,cmdnote)
			cmd = 'reboot'
			__tel.Execute(cmd,3)
			del __tel
			__tel = None
		except:
			raise RemoteConnectError('can not connect [%s:%s]=>[user:%s;password:%s]'%(host,port,user,password))
		finally:
			if __tel:
				del __tel
			__tel = None
		return


	def notest_A1RebootCase(self):
		# first to reboot 
		utcfg = xunit.config.XUnitConfig()
		origport = utcfg.GetValue('.sdkserver','port','30000')
		origport = int(origport)
		newport = origport + 301
		if newport >= 0xffff:
			newport = 1079
		ssock,sesid = self.SdkNetworkPortLoginIn()
		netport = ssock.GetNetworkPort()
		netport.SdkPort(newport)
		ssock.SetNetworkPort(netport)
		utcfg.SetValue('.sdkserver','port','%s'%(newport),1)
		port = utcfg.GetValue('.sdkserver','port','30000')

		ssock.CloseSocket()
		del ssock
		ssock = None
		
		self.__RebootRemote()
		sys.stdout.write('R')
		sys.stdout.flush()
		# we sleep for not connect 
		for i in xrange(20):
			time.sleep(1)
			sys.stdout.write('W')
			sys.stdout.flush()
		stime = time.time()
		etime = stime + 80
		ctime = stime
		ssock = None
		while ctime < etime:
			try:
				ssock,sesid = self.SdkStreamLoginIn()
				break
			except:
				if ssock:
					del ssock
				ssock = None
				time.sleep(1)
				sys.stdout.write('.')
				sys.stdout.flush()
			ctime = time.time()
		self.assertTrue( ssock is not None)
		# we should start stream 0
		ssock.StartStream([0])
		ssock.CloseSocket()
		del ssock
		ssock = None

		ssock,sesid = self.SdkNetworkPortLoginIn()
		netport = ssock.GetNetworkPort()
		netport.SdkPort(origport)
		ssock.SetNetworkPort(netport)
		utcfg.SetValue('.sdkserver','port','%s'%(origport),1)
		
		# now we should 
		self.__RebootRemote()
		sys.stdout.write('r')
		sys.stdout.flush()
		# we sleep for not connect 
		for i in xrange(20):
			time.sleep(1)
			sys.stdout.write('w')
			sys.stdout.flush()
		stime = time.time()
		etime = stime + 80
		ctime = stime
		ssock = None
		while ctime < etime:
			try:
				ssock,sesid = self.SdkStreamLoginIn()
				break
			except:
				if ssock:
					del ssock
				ssock = None
				time.sleep(1)
				sys.stdout.write('.')
				sys.stdout.flush()
			ctime = time.time()
		self.assertTrue( ssock is not None)
		# we should start stream 0
		ssock.StartStream([0])
		ssock.CloseSocket()
		del ssock
		ssock = None
		return 





class Login1Unit(SdkLoginUnit):
	def __SdkLoginUser(self,username,password):
		utcfg = xunit.config.XUnitConfig()
		host = utcfg.GetValue('.sdkserver','host','')
		port = utcfg.GetValue('.sdkserver','port','30000')
		port = int(port)
		username = str(username)
		password = str(password)
		host = str(host)		
		ssock = xunit.extlib.sdksock.SdkStreamSock(host,port)
		sesid = ssock.LoginUserPass(username,password)
		return ssock,sesid
		

	def __SdkLoginSession(self,sesid):
		utcfg = xunit.config.XUnitConfig()
		host = utcfg.GetValue('.sdkserver','host','')
		port = utcfg.GetValue('.sdkserver','port','30000')
		port = int(port)
		host = str(host)
		ssock = xunit.extlib.sdksock.SdkStreamSock(host,port)
		sesid = ssock.LoginSessionId(sesid)
		return ssock,sesid
		

	def test_A2SessionLogin(self):
		ssock,sesid = self.SdkStreamLoginIn()
		self.assertEqual(ssock.KeepTimeMs(),15000)
		ssock.CloseSocket()
		del ssock
		ssock = None
		ssock,sesid = self.__SdkLoginSession(sesid)
		return

	def test_A3LoggingFull(self):
		sockarr = []

		for i in xrange(20):			
			ssock,sesid = self.SdkStreamLoginIn()
			sockarr.append(ssock)

		ok =1
		try:
			ssock,sesid = self.SdkStreamLoginIn()
		except:
			ok = 0

		self.assertEqual(ok,0)
		for s in sockarr:
			s.CloseSocket()

		# sleep for a while ,and this will not give the session close ok on server
		time.sleep(1)
		return


class StartLogginSessionUnit(SdkLoginUnit):
	def __PreStopAllProcess(self):
		stime = time.time()
		etime = stime + 5
		ctime = stime
		pids = []
		while ctime < etime:
			pids = self.FindPids(['python','sdk_login.py'],['/bin/sh'])
			if len(pids) == 0:
				break
			self.KillPids(2,pids)
			time.sleep(0.2)
			ctime = time.time()
		if len(pids) != 0:
			raise ProcessStillRunningError('process of sdk_login.py still running %s'%(repr(pids)))
		return



	def test_A4StartLogginSessionTimeout(self):
		# now first to start
		self.__PreStopAllProcess()
		filed = os.path.dirname(os.path.abspath(__file__))
		utcfg = xunit.config.XUnitConfig()
		host = utcfg.GetValue('.sdkserver','host','')
		port = utcfg.GetValue('.sdkserver','port','30000')
		username = utcfg.GetValue('.sdkserver','username',None)
		password = utcfg.GetValue('.sdkserver','password',None)
		host = str(host)
		port = int(port)
		username = str(username)
		password = str(password)
		ssock,sesid = self.SdkStreamLoginIn()
		ssock.CloseSocket()
		del ssock
		ssock = None

		__seswaitfile = filed
		__seswaitfile += os.sep
		__seswaitfile += 'endtime.txt'
		
		cmd = 'python %s'%(filed)
		cmd += os.sep
		cmd += 'sdk_login.py  '
		cmd += ' -c '
		cmd += ' -H %s '%(host)
		cmd += ' -p %d '%(port)
		cmd += ' -s %d '%(sesid)
		cmd += '  2>%s'%(__seswaitfile)
		
		pid =  self.CallProcessStart(cmd)
		self.assertTrue( pid > 0 )
		utcfg.SetValue('.sdkserver','session_test_pid',str(pid))
		utcfg.SetValue('.sdkserver','session_test_time',str(time.time()))
		utcfg.SetValue('.sdkserver','session_test_write_file',__seswaitfile)

		retpid = utcfg.GetValue('.sdkserver','session_test_pid','0')
		rettime = utcfg.GetValue('.sdkserver','session_test_time','0')
		retfile = utcfg.GetValue('.sdkserver','session_test_write_file','')

		return

class BaseSdkLoginUnit(SdkLoginUnit):
	def test_B1LoginSessionTimeout(self):
		ssock,sesid = self.SdkStreamLoginIn()
		ssock.CloseSocket()
		ssock = None
		
		utcfg = xunit.config.XUnitConfig()
		ktimeout = utcfg.GetValue('.sdkserver','keeptime','15')
		ktimeout = int(ktimeout)

		for i in xrange(ktimeout + 5):
			sys.stdout.write('W')
			sys.stdout.flush()
			time.sleep(1)

		ok =1
		try:
			ssock,sesid = self.__SdkLoginSession(sesid)
		except:
			ok = 0

		self.assertEqual(ok,0)
		return

	def test_B2LoginIncorrectPassword(self):
		utcfg = xunit.config.XUnitConfig()
		username = utcfg.GetValue('.sdkserver','username',None)
		password = utcfg.GetValue('.sdkserver','password',None)
		username = str(username)
		password = str(password)
		if password is None:
			password = 'none'
		else:
			password += 'none'

		ok = 1
		try:
			ssock,sesid = self.__SdkLoginUser(username,password)
		except:
			ok = 0

		self.assertEqual(ok,0)
		return

	def test_B3LoginConfdirect(self):
		ssock,sesid = self.SdkStreamLoginIn()
		ssock.CloseSocket()
		ssock = None

		utcfg = xunit.config.XUnitConfig()
		host = utcfg.GetValue('.sdkserver','host','')
		port = utcfg.GetValue('.sdkserver','port','30000')
		host =str(host)
		port = int(port)
		ptzssock = xunit.extlib.sdksock.SdkPtzSock(host,port)
		ptzssock.SessionId(sesid)
		ptzssock.UpCmd(100)
		ptzssock.StopCmd()
		ptzssock.CloseSocket()
		del ptzssock
		ptzssock = None
		return

	def test_B4ConnectIncorrectData(self):
		utcfg = xunit.config.XUnitConfig()
		host = utcfg.GetValue('.sdkserver','host','')
		port = utcfg.GetValue('.sdkserver','port','30000')
		host = str(host)
		port = int(port)
		sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
		# now connect to the server
		sock.connect((host,port))
		sock.send('ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss')
		rlen = 0
		rbuf = ''
		while len(rbuf) < 7:
			rbuf += sock.recv(8192)

		self.assertEqual(rbuf,'GMI0300')
		sock.close()
		del sock
		sock = None
		return

	def test_B5ConnectJustClose(self):
		utcfg = xunit.config.XUnitConfig()
		host = utcfg.GetValue('.sdkserver','host','')
		port = utcfg.GetValue('.sdkserver','port','30000')
		host = str(host)
		port = int(port)
		sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
		sock.close()

		time.sleep(0.5)
		ssock,sesid = self.SdkStreamLoginIn()
		ssock.CloseSocket()
		ssock = None

		return

	def __GetPassKey(self,hwaddr):
		sa = hwaddr.split(':')
		passkey = ''
		for s in sa:
			n = int(s,16)
			passkey += chr(n)

		if len(passkey) < 8 :
			passkey += '\0' * (8-len(passkey))
		else:
			passkey = passkey[:8]
		return passkey

	def __GetNotInUsers(self,userinfos,addusers,username):
		rndusername = ''
		while True:
			rndcnt = random.randint(0,300000)
			rndusername = username + str(rndcnt)
			found = 0
			for u in userinfos:
				if u.UserName() == rndusername:
					found = 1
					break
			for u in addusers:
				if u.UserName() == rndusername:
					found = 1
					break
			if found == 0 :
				break
		return rndusername

	def test_B6GetLogUserInfo(self):
		utcfg = xunit.config.XUnitConfig()
		username = utcfg.GetValue('.sdkserver','username','')
		username = str(username)
		ssock,sesid = self.SdkUserInfoLoginIn()
		addusers = []
		try:
			# now we should get the userinfo
			userinfos = ssock.GetUserInfo()
			if len(userinfos) < 20:
				# because 8 users is enough length for large packet ,so we need this
				netinfos = ssock.GetIpInfo()
				hwaddr = netinfos[0].HwAddr()
				passkey = self.__GetPassKey(hwaddr)
				lusers = len(userinfos)
				for i in xrange(lusers,20):
					rndusername = self.__GetNotInUsers(userinfos,addusers,username)
					userinfo = sdkproto.userinfo.UserInfo(passkey)
					userinfo.UserName(rndusername)
					userinfo.UserPass(rndusername)
					userinfo.UserFlag(1)
					userinfo.UserLevel(0)
					ssock.SetUserInfo(userinfo)
					addusers.append(userinfo)
					sys.stdout.write('A')
					sys.stdout.flush()

			userinfos = ssock.GetUserInfo()
			if len(userinfos) < 20:
				raise UserNotInsertError('only get %d userinfos'%(len(userinfos)))
			for u in addusers:
				found = 0
				for us in userinfos:
					if us.UserName() == u.UserName():
						found = 1
						break
				if found == 0:
					raise UserNotInsertError('user add %s not insert'%(u.UserName()))
		finally:
			for u in addusers:
				try:
					ssock.DelUserInfo(u)
					sys.stdout.write('D')
					sys.stdout.flush()
				except:
					pass
			addusers = []

		return

	def __User30Info(self):
		utcfg = xunit.config.XUnitConfig()
		username = utcfg.GetValue('.sdkserver','username','')
		username = str(username)
		ssock,sesid = self.SdkUserInfoLoginIn()
		addusers = []
		modusers = []
		oldusers = []

		try:
			userinfos = ssock.GetUserInfo()
			oldusers  = userinfos

			for u in userinfos:
				if u.UserName() != username:
					newuser = sdkproto.userinfo.UserInfo(u)
					rndpassword = self.__GetNotInUsers(userinfos,addusers,username)					
					newuser.UserPass(rndpassword)
					modusers.append(newuser)
			if len(modusers) < 30:
				netinfos = ssock.GetIpInfo()
				hwaddr = netinfos[0].HwAddr()
				passkey = self.__GetPassKey(hwaddr)
				lusers = len(userinfos)
				while len(modusers) < 30:
					rndusername = self.__GetNotInUsers(userinfos,addusers,username)
					userinfo = sdkproto.userinfo.UserInfo(passkey)
					userinfo.UserName(rndusername)
					userinfo.UserPass(rndusername)
					userinfo.UserFlag(1)
					userinfo.UserLevel(0)
					modusers.append(userinfo)
					addusers.append(userinfo)
					sys.stdout.write('%d'%(len(modusers)))
					sys.stdout.flush()
			# now we should set user			
			sys.stdout.write('%d'%(len(modusers)))
			sys.stdout.flush()
			ssock.SetUserInfoTimeout(modusers,40.0)

			# now to get for the name
			userinfos = ssock.GetUserInfo()
			for u in modusers:
				findone=0
				for u2 in userinfos:
					if u.UserName() == u2.UserName() and \
						u.UserPass() == u2.UserPass() and \
						u.UserFlag() == u2.UserFlag() and \
						u.UserLevel() == u2.UserLevel():
						findone = 1
						sys.stdout.write('C')
						sys.stdout.flush()
						break
				self.assertEqual(findone,1)
		finally:
			errors = 0
			for u in addusers:
				try:
					ssock.DelUserInfo(u)
					sys.stdout.write('D')
					sys.stdout.flush()
				except:
					#errors += 1
					sys.stdout.write('e')
					sys.stdout.flush()
					pass

			for u in oldusers:
				try:
					if u.UserName() != username:
						ssock.SetUserInfo(u)
						sys.stdout.write('R')
						sys.stdout.flush()
				except:
					#errors += 1
					sys.stdout.write('E')
					sys.stdout.flush()
					pass		
			addusers = []
			oldusers = []
			self.assertEqual(errors,0)
		time.sleep(2.0)
		return
	def test_B7LongUserInfoSend(self):
		for i in xrange(10):
			self.__User30Info()
		return
		

	def test_B8LoginHeartbeatovertime(self):
		ssock,sesid = self.SdkStreamLoginIn()
		ssock.CloseSocket()
		ssock = None

		# now to make heart beat login
		ssock,newsesid = self.SdkSessionHeartBeatLogin(sesid)
		for i in xrange(20):
			time.sleep(1)
			sys.stdout.write('.')
			sys.stdout.flush()
		# now it is timeout ,so we should make error
		ok = 1
		try:
			ssock.LoginHeartBeatTimeout(sesid,1);
		except xunit.extlib.sdksock.SdkSockRecvError as e:
			ok =0
		self.assertEqual(ok,0)

		ssock.CloseSocket()
		ssock = None

		# now we should make sure that the reconnect is ok
		ssock,sesid = self.SdkStreamLoginIn()
		ssock.CloseSocket()
		ssock = None
		
		return


class BaseSdkStreamUnit(SdkLoginUnit):

	def __KillStreams(self):
		stime = time.time()
		etime = stime + 5
		ctime = stime
		pids = []
		exfilters = ['/bin/sh']
		while ctime < etime:
			pids = self.FindPids(['sdk_stream.py'],['/bin/sh'])
			if len(pids) == 0 :
				break
			self.KillPids(2,pids)
			time.sleep(0.2)
			ctime = time.time()
		if len(pids) != 0:
			raise ProcessStillRunningError('process sdk_stream.py still running %s'%(repr(pids)))			
		return
	def test_C100StreamGet(self):
		self.__KillStreams()
		ssock,sesid = self.SdkStreamLoginIn()
		ssock.CloseSocket()
		ssock = None
		utcfg = xunit.config.XUnitConfig()
		host = utcfg.GetValue('.sdkserver','host','')
		port = utcfg.GetValue('.sdkserver','port','30000')
		host =str(host)
		port = int(port)
		streamsock = xunit.extlib.sdksock.SdkStreamSock(host,port)
		streamsock.SessionId(sesid)
		streamsock.StartStream(['0'])

		stime = time.time()
		etime = int(stime + 30)
		ctime = int(stime)
		lastvidx = None
		lastaidx = None
		curidx = None
		lctime = ctime
		while ctime < etime:
			typepack = streamsock.GetStreamPacket()
			ctime = time.time()
			ctime = int(ctime)
			if lctime != ctime:
				lctime = ctime
			if typepack == sdkproto.pack.GMIS_PROTOCOL_TYPE_MEDIA_CTRL:
				continue
			frametype = streamsock.GetFrameType()
			self.assertTrue(frametype == 'I' or frametype == 'A' or frametype == 'P')
			curidx = streamsock.GetStreamIdx()
			if frametype == 'I' or frametype == 'P':
				if lastvidx is None:
					self.assertTrue(frametype == 'I')
				else:
					self.assertTrue( ((lastvidx+1) == curidx) or (curidx == 0 and lastvidx == 0xffffffff))
				lastvidx = curidx
				if (lastvidx % 100) == 0:
					sys.stdout.write('V')
					sys.stdout.flush()
			elif frametype == 'A':
				if lastaidx is not None:
					if not (((lastaidx+1) == curidx) or (curidx == 0 and lastaidx == 0xffffffff)):
						logging.error('lastaidx %d curidx %d'%(lastaidx,curidx))
					self.assertTrue( ((lastaidx+1) == curidx) or (curidx == 0 and lastaidx == 0xffffffff))
				lastaidx = curidx
				if (lastaidx %100) == 0:
					sys.stdout.write('A')
					sys.stdout.flush()
			ctime = time.time()
			

		streamsock.CloseSocket()
		streamsock = None

		return


	def __RunStream(self,streamid,count=500):
		utcfg = xunit.config.XUnitConfig()
		host = utcfg.GetValue('.sdkserver','host','')
		port = utcfg.GetValue('.sdkserver','port','30000')
		username = utcfg.GetValue('.sdkserver','username',None)
		password = utcfg.GetValue('.sdkserver','password',None)
		port = int(port)
		username = str(username)
		password = str(password)
		host = str(host)		
		
		filed = os.path.dirname(os.path.abspath(__file__))

		cmd = 'python %s'%(filed)
		cmd += os.sep
		cmd += 'sdk_stream.py'
		cmd += ' -H %s '%(host)
		cmd += ' -p %d '%(port)
		cmd += ' -u %s '%(username)
		cmd += ' -P %s '%(password)
		cmd += ' -c %d '%(count)
		cmd += ' %d '%(streamid)

		return self.CallProcessStart(cmd)

	def __RunSleepStream(self,streamid,slptime,slppercent,count=1000):
		utcfg = xunit.config.XUnitConfig()
		host = utcfg.GetValue('.sdkserver','host','')
		port = utcfg.GetValue('.sdkserver','port','30000')
		username = utcfg.GetValue('.sdkserver','username',None)
		password = utcfg.GetValue('.sdkserver','password',None)
		port = int(port)
		username = str(username)
		password = str(password)
		host = str(host)		
		
		filed = os.path.dirname(os.path.abspath(__file__))

		cmd = 'python %s'%(filed)
		cmd += os.sep
		cmd += 'sdk_stream.py'
		cmd += ' -H %s '%(host)
		cmd += ' -p %d '%(port)
		cmd += ' -u %s '%(username)
		cmd += ' -P %s '%(password)
		cmd += ' -t %f '%(slptime)
		cmd += ' -r %d '%(slppercent)
		cmd += ' -c %d '%(count)
		cmd += ' %d '%(streamid)

		return self.CallProcessStart(cmd)
		

	def test_C101Stream5Streams(self):
		self.__KillStreams()

		streampids = []
		for i in xrange(5):
			if i % 2 == 0:
				streamid = 0
			else:
				streamid = 1
			streampids.append(self.__RunStream(streamid))

		stime = time.time()
		etime = stime + 300
		ctime = stime
		while ctime < etime:
			streampids,removepid,retval = self.WaitPidsReturn(streampids)
			if removepid != 0 :
				if retval != 0:
					raise ProcessRunningError('pid(%d) exit (%d)'%(removepid,retval))
			
			if len(streampids) == 0:
				break
			sys.stdout.write('%d'%(len(streampids)))
			sys.stdout.flush()
			time.sleep(1.0)
			ctime = time.time()
		if len(streampids) > 0:
			raise ProcessStillRunningError('still %s running'%(repr(streampids)))
		return

	def test_C102StreamSleep(self):
		self.__KillStreams()
		streampids = []

		for i in xrange(6):
			if i %2 == 0 :
				streamid = 0
				slptime = 0.2
				slppercent = 30
			else:
				streamid = 1
				slptime = 0.3
				slppercent = 20
			streampids.append(self.__RunSleepStream(streamid,slptime,slppercent))

		stime = time.time()
		etime = stime + 500
		ctime = stime

		while ctime < etime:
			streampids,removepid,retval = self.WaitPidsReturn(streampids)
			if removepid != 0:
				if retval != 0 :
					raise ProcessRunningError('stream %d retval(%d)'%(removepid,retval))
			if len(streampids) == 0:
				break
			sys.stdout.write('%d'%(len(streampids)))
			sys.stdout.flush()
			time.sleep(1.0)
			ctime = time.time()
		if len(streampids) != 0:
			raise ProcessStillRunningError('stream %s still running'%(repr(streampids)))

		return

	def test_C103StreamRecvTimeout(self):
		self.__KillStreams()
		ssock,sesid = self.SdkStreamLoginIn()
		ssock.CloseSocket()
		ssock = None
		utcfg = xunit.config.XUnitConfig()
		host = utcfg.GetValue('.sdkserver','host','')
		port = utcfg.GetValue('.sdkserver','port','30000')
		host =str(host)
		port = int(port)
		streamsock = xunit.extlib.sdksock.SdkStreamSock(host,port)
		streamsock.SessionId(sesid)
		streamsock.StartStream(['0'])

		stime = time.time()
		etime = stime + 5
		ctime = stime
		lastaidx = None
		lastvidx = None
		curidx = None
		lctime = int(ctime)
		while ctime < etime:
			typepack = streamsock.GetStreamPacket()
			ctime = time.time()
			ctime = int(ctime)
			if lctime != ctime:
				lctime = ctime
				sys.stdout.write('R')
				sys.stdout.flush()
			if typepack == sdkproto.pack.GMIS_PROTOCOL_TYPE_MEDIA_DATA:
				continue

			frametype = streamsock.GetStreamType()
			curidx = streamsock.GetStreamIdx()
			if frametype == 'A':
				if lastaidx is None:
					lastaidx = curidx
				else:
					if lastaidx != 0xffffffff:
						self.assertEqual(lastaidx+1,curidx)
					else:
						self.assertEqual(lastaidx,0)
					lastaidx = curidx
			else:
				self.assertTrue( frametype == 'I' or frametype == 'P')
				if lastvidx is None:
					self.assertEqual(frametype,'I')
					lastvidx = curidx
				else:					
					if lastvidx != 0xffffffff:
						self.assertEqual(lastvidx+1,curidx)
					else:
						self.assertEqual(lastvidx,0)
					lastvidx = curidx

		for i in xrange(20):
			time.sleep(1)
			sys.stdout.write('.')
			sys.stdout.flush()
		ok = 1
		stime = time.time()
		etime = stime + 5
		ctime = stime
		try:
			while ctime < etime:
				typepack = streamsock.GetStreamPacket()
				ctime = time.time()
		except:
			ok = 0
		self.assertEqual(ok,0)

		streamsock.CloseSocket()
		del streamsock
		streamsock = None
		return

	def test_C104StreamAudioMonitor(self):
		# now to get the stream audio monitor
		videocfgsock,sesid = self.SdkVideoCfgLogin()
		videos = videocfgsock.GetVideoCfg()
		fv = None
		for v in videos:
			if v.Flag() == 0:
				fv = v
				break
		oldstreamtype = fv.StreamType()
		#logging.info('stream type %d'%(oldstreamtype))
		try:
			if oldstreamtype != 2:
				fv.StreamType(2)
				#logging.info('stream type %d'%(oldstreamtype))
				videocfgsock.SetVideoCfg(fv)
				#logging.info('stream type %d'%(oldstreamtype))
				secvideos = videocfgsock.GetVideoCfg()
				#logging.info('stream type %d'%(oldstreamtype))
				secfv = None
				for v in secvideos:
					if v.Flag() == 0:
						secfv = v
						break
				self.assertTrue( secfv is not None )
				#logging.info('stream type %d'%(secfv.StreamType()))
			# now we should get the stream

			streamsock,sesid = self.SdkStreamLoginIn()
			streamsock.StartStream(['0'])
			audiopacks = 0
			videopacks = 0
			lastvidx = None
			lastaidx = None
			stime = time.time()
			ctime = stime
			etime = stime + 30
			while ctime < etime:
				packtype = streamsock.GetStreamPacket()
				if packtype != xunit.extlib.sdkproto.pack.GMIS_PROTOCOL_TYPE_MEDIA_DATA:
					ctime = time.time()
					continue
				frametype = streamsock.GetFrameType()
				self.assertTrue(frametype == 'I' or frametype == 'P' or frametype == 'A')
				curidx = streamsock.GetStreamIdx()
				if frametype == 'P' or frametype == 'I':					
					if lastvidx is None:
						self.assertTrue(frametype == 'I')
					else:
						#logging.info('lastvidx %d curidx %d'%(lastvidx,curidx))
						self.assertTrue( (lastvidx + 1) == curidx or (lastvidx == 0xffffffff and curidx == 0) )
					lastvidx = curidx
					videopacks += 1
					if (videopacks % 100) == 0:
						sys.stdout.write('V')
						sys.stdout.flush()
				else:
					if lastaidx is not None:
						self.assertTrue( (lastaidx + 1) == curidx or (lastaidx == 0xffffffff and curidx == 0))
					lastaidx = curidx
					audiopacks += 1
					if (audiopacks % 100) == 0:
						sys.stdout.write('A')
						sys.stdout.flush()
				ctime = time.time()
			# we assume that we have audio packs and video packs for it
			self.assertTrue(videopacks > 100)
			self.assertTrue(audiopacks > 100)
			streamsock.CloseSocket()
		finally:
			if oldstreamtype != 2:
				fv.StreamType(oldstreamtype)
				videocfgsock.SetVideoCfg(fv)
			videocfgsock.CloseSocket()
		time.sleep(2.0)
		return

	def __ReceiveAudioData(self,ssock,retobj):
		if not hasattr(retobj,'running'):
			raise InvalidParameterError('not set retobj.running')

		if not hasattr(retobj,'rcvexited'):
			raise InvalidParameterError('not set retobj.rcvexited')

		if not hasattr(retobj,'audiorcvd'):
			raise InvalidParameterError('not set retobj.audiorcvd')

		if not hasattr(retobj,'audiotime'):
			raise InvalidParameterError('not set retobj.audiotime')
		if not hasattr(retobj,'rcvres'):
			raise InvalidParameterError('not set retobj.rcvres')

		

		audiotime = float(retobj.audiotime)
		lastaidx = None
		lastvidx = None
		retobj.audiorcvd = 0

		try:
			while retobj.running == 1:
				data = ssock.ReceiveData(audiotime)
				framepack = ssock.GetReceiveHdr()
				if lastaidx is not None:
					expectidx = lastaidx
					expectidx += 1
					if expectidx > 0xffffffff:
						expectidx = 0
					if framepack.FrameId() != expectidx:
						logging.error('frameid %d != expectidx %d'%(framepack.FrameId(),expectidx))
						raise Exception('frameid %d != expectidx %d'%(framepack.FrameId(),expectidx))
				lastaidx = framepack.FrameId()
				if (lastaidx % 100) == 0:
					sys.stdout.write('R')
					sys.stdout.flush()
				retobj.audiorcvd += 1
			retobj.rcvres=0
			retobj.rcvexited = 1
		except:
			traceback.print_exc()
			retobj.rcvexited = 1
			retobj.rcvres = 2
		return

	def __SendAudioData(self,ssock,framepack,retobj):
		if not hasattr(retobj,'running'):
			raise InvalidParameterError('not set retobj.running')

		if not hasattr(retobj,'sndexited'):
			raise InvalidParameterError('not set retobj.sndexited')

		if not hasattr(retobj,'audiosnd'):
			raise InvalidParameterError('not set retobj.audiosnd')

		if not hasattr(retobj,'audiotime'):
			raise InvalidParameterError('not set retobj.audiotime')
		if not hasattr(retobj,'sndres'):
			raise InvalidParameterError('not set retobj.sndres')
		data = '\xd5' * 160
		curpts = 0
		curframeid =0
		MAX_LONGLONGS = (1<<64) - 1
		MAX_INTS = (1<<32) -1
		data = '\xd5' * 1000
		curpts = 0
		curframeid =0
		MAX_LONGLONGS = (1<<64) - 1
		MAX_INTS = (1<<32) -1
		try:
			while retobj.running == 1:
				ssock.SendData(framepack,data)
				curpts += 11250
				curframeid += 1
				if curpts > MAX_LONGLONGS:
					curpts %= MAX_LONGLONGS
				if curframeid > MAX_INTS:
					curframeid %= MAX_INTS
				framepack.Pts(curpts)
				framepack.FrameId(curframeid)
				time.sleep(0.125)
				retobj.audiosnd += 1
				if (curframeid % 20)==0:
					sys.stdout.write('S')
					sys.stdout.flush()
			retobj.sndexited = 1
			retobj.sndres = 0
		except:
			traceback.print_exc()
			retobj.sndexited = 1
			retobj.sndres = 2
		return
		
	
	def test_C105AudioDualBasic(self):
		# now to test for the audio coder
		ssock,sesid = self.SdkAudioDualLogin()
		framepack = sdkproto.audiodual.AudioFramePack()
		startreq = sdkproto.audiodual.StartTalkRequest()
		resp = ssock.StartAudioDual(startreq)
		retobj = AudioThreadCtrl()
		thread.start_new_thread(self.__ReceiveAudioData,(ssock,retobj,))
		thread.start_new_thread(self.__SendAudioData,(ssock,framepack,retobj,))
		stime = time.time()
		etime = stime + 30
		ctime = stime
		while ctime < etime:
			time.sleep(1.0)			
			ctime = time.time()
		retobj.running = 0
		while retobj.rcvexited == 0 or retobj.sndexited == 0:
			time.sleep(0.1)
		ssock.CloseSocket()
		self.assertTrue( retobj.sndres == 0)
		self.assertTrue( retobj.rcvres == 0)
		self.assertTrue( retobj.audiorcvd > 100)
		self.assertTrue( retobj.audiosnd > 100)
		time.sleep(2.0)
		return

	def test_C106nodoubleAudioDual(self):
		# we sleep a while for free things ok
		time.sleep(2.0)
		ssock,sesid = self.SdkAudioDualLogin()
		framepack = sdkproto.audiodual.AudioFramePack()
		startreq = sdkproto.audiodual.StartTalkRequest()
		resp = ssock.StartAudioDual(startreq)
		retobj = AudioThreadCtrl()
		thread.start_new_thread(self.__ReceiveAudioData,(ssock,retobj,))
		thread.start_new_thread(self.__SendAudioData,(ssock,framepack,retobj,))
		ok =1
		try:
			ssock2,sesid = self.SdkAudioDualLogin()
			ssock2.StartAudioDual(startreq)
		except:
			ok = 0

		self.assertEqual(ok,0)

		retobj.running=0
		while retobj.rcvexited == 0 or retobj.sndexited == 0:
			time.sleep(0.1)
		ssock.CloseSocket()
		ssock2.CloseSocket()
		time.sleep(2.0)
		return

	def test_C107Login1000Test(self):
		count = 0
		ssock = None
		try:
			while count < 1000:
				self.assertTrue(ssock is None)
				ssock,sesid = self.SdkVideoCfgLogin()
				ssock.CloseSocket()
				ssock = None
				if (count % 50) == 0:
					sys.stdout.write('C')
					sys.stdout.flush()
				count += 1				
		finally:
			if ssock is not None:
				ssock.CloseSocket()
			ssock = None
		return

	def test_C108ChangeConfigReconnect(self):
		ssock = None
		oldstreamtype = None
		try:
			ssock,sesid = self.SdkVideoCfgLogin()
			vcfgs = ssock.GetVideoCfg()
			oldstreamtype = vcfgs[0].StreamType()
			newstreamtype = 2
			if oldstreamtype == 2:
				newstreamtype = 1
			vcfgs[0].StreamType(newstreamtype)
			ssock.SetVideoCfg(vcfgs[0])
			ssock.CloseSocket()
			ssock = None
			ssock,sesid = self.SdkStreamLoginIn()
			ssock.StartStream([0])
			ssock.CloseSocket()
			ssock = None
		finally:
			ok = 1
			if oldstreamtype is not None:
				try:
					ss2 ,sesid = self.SdkVideoCfgLogin()
					vcfgs = ss2.GetVideoCfg()
					vcfgs[0].StreamType(oldstreamtype)
					ss2.SetVideoCfg(vcfgs[0])
					ss2.CloseSocket()
					ss2 = None
				except:
					ok = 0
			oldstreamtype = None
			if ssock is not None:
				ssock.CloseSocket()
			ssock = None
			self.assertEqual(ok,1)

		return

	def test_C109AudioVideoPauseResumeCase(self):
		ssock = None
		oldstreamtype = None
		streampids = []
		self.__KillStreams()
		oldcfg = None
		try:
			ssock,sesid = self.SdkVideoCfgLogin()
			vcfgs = ssock.GetVideoCfg()
			oldcfgbuf = vcfgs[0].Format()
			oldcfg = sdkproto.videocfg.EncodeCfg(oldcfgbuf)
			oldstreamtype = vcfgs[0].StreamType()
			newstreamtype = 2
			if oldstreamtype != 2:
				vcfgs[0].StreamType(newstreamtype)
				ssock.SetVideoCfg(vcfgs[0])
			else:			
				oldstreamtype = None
			ssock.CloseSocket()
			ssock = None

			# now we should call the process
			streampids.append(self.__RunStream(0,500))
			# now we should config 
			ssock,sesid = self.SdkVideoCfgLogin()
			vcfgs = ssock.GetVideoCfg()
			quality = vcfgs[0].Quality()
			if quality != 3:
				quality = 3
			else:
				quality = 4
			vcfgs[0].Quality(quality)
			ssock.SetVideoCfg(vcfgs[0])
			ssock.CloseSocket()
			ssock = None
			stime = time.time()
			etime = stime + 100
			ctime = stime
			while ctime < etime:
				streampids,removepid,retval = self.WaitPidsReturn(streampids)
				if removepid != 0 :
					if retval != 0:
						raise ProcessRunningError('pid(%d) exit (%d)'%(removepid,retval))
				
				if len(streampids) == 0:
					break
				sys.stdout.write('%d'%(len(streampids)))
				sys.stdout.flush()
				time.sleep(1.0)
				ctime = time.time()

			if ctime >= etime:
				raise ProcessStillRunningError('%d still running'%(streampids[0]))

			# now we wait for the stream handle
			
		finally:
			self.__KillStreams()
			ok = 1
			if oldcfg is not None:
				try:
					ss2 ,sesid = self.SdkVideoCfgLogin()
					ss2.SetVideoCfg(oldcfg)
					ss2.CloseSocket()
					ss2 = None
				except:
					ok = 0
			oldstreamtype = None
			if ssock is not None:
				ssock.CloseSocket()
			ssock = None
			self.assertEqual(ok,1)
		return
	def __KillAlarm(self):
		stime = time.time()
		etime = stime + 5
		ctime = stime
		pids = []
		while ctime < etime:
			pids = self.FindPids(['sdk_alarm.py'],['/bin/sh'])
			if len(pids) == 0 :
				break
			self.KillPids(2,pids)
			time.sleep(0.2)
			ctime = time.time()
		if len(pids) != 0:
			raise ProcessStillRunningError('process sdk_alarm.py still running %s'%(repr(pids)))			
		return
		

	def __RunAlarm(self,wfile):
		utcfg = xunit.config.XUnitConfig()
		host = utcfg.GetValue('.sdkserver','host','')
		port = utcfg.GetValue('.sdkserver','port','30000')
		username = utcfg.GetValue('.sdkserver','username',None)
		password = utcfg.GetValue('.sdkserver','password',None)
		port = int(port)
		username = str(username)
		password = str(password)
		host = str(host)		
		
		filed = os.path.dirname(os.path.abspath(__file__))

		cmd = 'python %s'%(filed)
		cmd += os.sep
		cmd += 'sdk_alarm.py'
		cmd += ' -H %s '%(host)
		cmd += ' -p %d '%(port)
		cmd += ' -u %s '%(username)
		cmd += ' -P %s '%(password)
		cmd += ' --write %s '%(wfile)
		return self.CallProcessStart(cmd)

	def __CheckAlarm(self,wfile,wid,wtype,wlevel,wonoff,wtime,wdevid,wdesc):
		fp = open(wfile,'r+b')
		lines = fp.readlines()
		fp.close()
		fp = None
		matchid=0
		matchtype=0
		matchlevel=0
		matchonoff=0
		matchtime=0
		matchdevid=0
		matchdesc=0
		pat = re.compile('[\w\s]+:[\s]*\(([\w-]+)\)')
		sid = '%s'%(wid)
		stype = '%s'%(wtype)
		slevel = '%s'%(wlevel)
		sonoff = '%s'%(wonoff)
		stime = '%s'%(wtime)
		sdevid = '%s'%(wdevid)
		sdesc = '%s'%(wdesc)

		for l in lines:
			l = l.strip('\r\n')
			if l.startswith('warningid'):
				m = pat.match(l)
				s = m.group(1)
				if s == sid:
					matchid = 1
			elif matchid and matchtype == 0:
				self.assertTrue(l.startswith('warningtype'))
				m = pat.match(l)
				s = m.group(1)
				if s == stype:
					matchtype = 1
				else:
					matchid = 0
					matchtype = 0
					matchlevel = 0
					matchonoff = 0
					matchtime = 0
					matchdevid = 0
					matchdesc = 0
			elif matchtype and matchlevel == 0:
				self.assertTrue(l.startswith('warninglevel'))
				m = pat.match(l)
				s = m.group(1)
				if s == slevel:
					matchlevel = 1
				else:
					matchid = 0
					matchtype = 0
					matchlevel = 0
					matchonoff = 0
					matchtime = 0
					matchdevid = 0
					matchdesc = 0
			elif matchlevel and matchonoff == 0:
				self.assertTrue(l.startswith('warningonoff'))
				m = pat.match(l)
				s = m.group(1)
				if s == sonoff:
					matchonoff = 1
				else:
					matchid = 0
					matchtype = 0
					matchlevel = 0
					matchonoff = 0
					matchtime = 0
					matchdevid = 0
					matchdesc = 0
			elif matchonoff and matchtime == 0:
				self.assertTrue(l.startswith('time'))
				m = pat.match(l)
				s = m.group(1)
				if s == stime:
					matchtime = 1
				else:
					matchid = 0
					matchtype = 0
					matchlevel = 0
					matchonoff = 0
					matchtime = 0
					matchdevid = 0
					matchdesc = 0
			elif matchtime and matchdevid == 0:
				self.assertTrue(l.startswith('devid'))
				m = pat.match(l)
				s = m.group(1)
				if s == sdevid:
					matchdevid = 1
				else:
					matchid = 0
					matchtype = 0
					matchlevel = 0
					matchonoff = 0
					matchtime = 0
					matchdevid = 0
					matchdesc = 0
			elif matchdevid and matchdesc == 0:
				self.assertTrue(l.startswith('description'))
				m = pat.match(l)
				s = m.group(1)
				if s == sdesc:
					matchdesc = 1
					break
				else:
					matchid = 0
					matchtype = 0
					matchlevel = 0
					matchonoff = 0
					matchtime = 0
					matchdevid = 0
					matchdesc = 0
		self.assertTrue(matchdesc == 1)
			
		
		return

	def test_C110ToGetMessage(self):
		utcfg = xunit.config.XUnitConfig()
		buildtop = utcfg.GetValue('build','topdir','.')
		tmpdir = utcfg.GetValue('.telnet','tmpdir','/tmp')
		messagefile='%s/messageid.txt'%(tmpdir)
		random.seed(time.time())
		tmpid = random.randint(130,10000)
		tmptype = random.randint(20,3569)
		tmplevel = random.randint(35,111)
		tmponoff = random.randint(0,32)
		tmptime = time.strftime('%Y%m%d-%H%M%S',time.localtime(time.time()))
		tmpdevid = 'tmpdevid'
		tmpdescription = 'tmpdescription'
		shellcmd='echo "%d %d %d %d %s %s %s">%s'%(tmpid,tmptype,tmplevel,tmponoff,tmptime,tmpdevid,tmpdescription,messagefile)
		self.TelnetExecCmd(shellcmd)
		# now we should copy the message board
		self.Mountdir()
		alarmtest='%s/ipc_fw3.x_core/output/ambarella_a5s_sdk_v3.3/application/sdk_server/unittest/alarm/sdkalarm_unitest'%(buildtop)
		self.CopyNfsFile(alarmtest,tmpdir)
		# now we start the command
		writefile='%s/wmessagefile.txt'%(buildtop)
		# now we should run alarm
		self.__KillAlarm()
		alarmpid = self.__RunAlarm(writefile)
		# sleep a while ,and wait it ok
		time.sleep(0.5)

		shellcmd = '%s/sdkalarm_unitest %s'%(tmpdir,messagefile)
		self.TelnetExecCmd(shellcmd)

		time.sleep(0.6)
		self.__KillAlarm()
		self.__CheckAlarm(writefile,tmpid,tmptype,tmplevel,tmponoff,tmptime,tmpdevid,tmpdescription)
		shellcmd = 'rm -f %s/sdkalarm_unitest'%(tmpdir)
		self.TelnetExecCmd(shellcmd)
		shellcmd = 'rm -f %s'%(messagefile)
		self.TelnetExecCmd(shellcmd)		
		return

	def __prepare_leakout(self):
		self.__backcmd = None
		utcfg = xunit.config.XUnitConfig()
		buildtop = utcfg.GetValue('build','topdir','.')
		tmpdir = utcfg.GetValue('.telnet','tmpdir','/tmp')
		self.Mountdir()
		# now we should copy the file
		shellfile = '%s/ipc_fw3.x_core/prebuilt/tools/leak-check'%(buildtop)
		self.CopyNfsFile(shellfile,tmpdir)
		sofile = '%s/ipc_fw3.x_core/prebuilt/ambarella_a5s_sdk_v3.3/binary/libleaktracer.so'%(buildtop)
		self.CopyNfsFile(sofile,tmpdir)
		sdkfile = '%s/ipc_fw3.x_core/output/ambarella_a5s_sdk_v3.3/application/sdk_server/source/sdk_server'%(buildtop) 
		self.CopyNfsFile(sdkfile,tmpdir)
		self.RemoteKill('sdk_server')
		self.__backcmd = cfgexptel.CfgExpTel()
		self.__backcmd.Login()
		cmdstr = 'cd %s'%(tmpdir)
		self.__backcmd.Execute(cmdstr)
		cmdstr = 'pwd'
		matched ,result = self.__backcmd.Execute(cmdstr)
		#logging.info('curdir %s'%(result))
		cmdstr = 'ls'
		matched ,result = self.__backcmd.Execute(cmdstr)
		#logging.info('ls %s'%(result))
		cmdstr = './leak-check ./sdk_server 2>/dev/null'
		self.__backcmd.WriteLine(cmdstr)
		time.sleep(1.0)
		
		# now we should 
		forgestreamfile = '%s/ipc_fw3.x_core/output/ambarella_a5s_sdk_v3.3/application/sdk_server/unittest/forgestream/forgestream_unitest'%(buildtop)
		self.CopyNfsFile(forgestreamfile,tmpdir)
		# now to set the forge stream
		cmdtel = cfgexptel.CfgExpTel()
		cmdtel.Login()
		cmdstr = 'ps -ef | grep sdk_server | grep -v sdk_server'		
		matched ,getresult=cmdtel.Execute(cmdstr)
		#logging.info('result %s'%(getresult))
		cmdstr = '%s/forgestream_unitest --start /opt/config/gmi_setting.xml 3301'%(tmpdir)
		matched,result = cmdtel.Execute(cmdstr)
		#logging.info('cmd(%s) (%s)'%(cmdstr,result))
		cmdstr = 'netstat -nap | grep sdk'
		matched,result = cmdtel.Execute(cmdstr)
		#logging.info('cmd(%s) (%s)'%(cmdstr,result))		
		cmdtel.Logout()
		cmdtel = None
		time.sleep(1.0)
		return

	def __clean_leakout(self):
		self.RemoteKill('sdk_server')
		if self.__backcmd :
			self.__backcmd.Logout()
			self.__backcmd = None
		utcfg = xunit.config.XUnitConfig()
		tmpdir = utcfg.GetValue('.telnet','tmpdir','/tmp')
		
		cmdtel = cfgexptel.CfgExpTel()
		cmdtel.Login()
		cmdstr = 'rm -f %s/leak-check'%(tmpdir)
		cmdtel.Execute(cmdstr)
		cmdstr = 'rm -f %s/libleaktracer.so'%(tmpdir)
		cmdtel.Execute(cmdstr)
		cmdstr = 'rm -f %s/forgestream_unitest'%(tmpdir)
		cmdtel.Execute(cmdstr)
		cmdstr = 'rm -f %s/leak.out*'%(tmpdir)
		cmdtel.Execute(cmdstr)
		
		cmdtel.Logout()
		self.UnMountdir()
		return

	def __check_leakout(self):
		self.RemoteKill('sdk_server')
		if self.__backcmd :
			self.__backcmd.Logout()
			self.__backcmd = None

		utcfg = xunit.config.XUnitConfig()
		tmpdir = utcfg.GetValue('.telnet','tmpdir','/tmp')
		# now we should check for the leak.out file
		cmdtel = cfgexptel.CfgExpTel()
		cmdtel.Login()
		cmdstr = 'cat %s/leak.out | wc -l'%(tmpdir)
		matched,result = cmdtel.Execute(cmdstr)
		lines = int(result)
		cmdtel.Logout()
		cmdtel = None
		if lines >= 100:
			logging.error('line is %d'%(lines))
		self.assertTrue( lines < 100)		
		return


	def test_C998GetLogoutTimeout(self):
		utcfg = xunit.config.XUnitConfig()
		sessionpid = utcfg.GetValue('.sdkserver','session_test_pid',0)
		sessionstarttime = utcfg.GetValue('.sdkserver','session_test_time',0.0)
		sessionwritefile = utcfg.GetValue('.sdkserver','session_test_write_file','')
		pids = self.FindPids(['python','sdk_login.py','/bin/sh'],[])
		sessionpid = int(sessionpid)
		sessionwritefile = str(sessionwritefile)
		sessionstarttime = float(sessionstarttime)
		curtime = float(time.time())
		if len(pids) != 1:
			raise InvalidParameterError('len(%d)'%(len(pids)))
		if sessionpid != pids[0]:
			raise InvalidParameterError('sessionpid %d != pids[0](%d)'%(sessionpid,pids[0]))
		if sessionstarttime == 0.0:
			raise InvalidParameterError('sessionstarttime == 0.0')
		if (sessionstarttime + 1500) >= curtime:
			logging.warning('sessionstarttime (%f) + 1500 >= %f'%(sessionstarttime,curtime))
		return
		
	def test_C999SetNtpAddr4000(self):
		self.__KillStreams()
		self.__prepare_leakout()
		streampids = []
		try:
			for i in xrange(5):
				if i % 2 == 0:
					streamid = 0
				else:
					streamid = 1
				streampids.append(self.__RunStream(streamid,4000))
			lasttime=time.time()
			for i in xrange(4000):
				ssock,sesid = self.SdkTimeLogin()
				ssock.CloseSocket()
				ssock = None
				if (i%50) == 0:
					streampids,removepid,retval = self.WaitPidsReturn(streampids)
					if removepid != 0 :
						if retval != 0:
							raise ProcessRunningError('pid(%d) exit (%d)'%(removepid,retval))
					curtime = time.time()
					if len(streampids) > 0 :
						sys.stdout.write('%d'%(len(streampids)))
					else:
						break
					sys.stdout.flush()
					lasttime=curtime
			while len(streampids) > 0:
				streampids,removepid,retval = self.WaitPidsReturn(streampids)
				if removepid != 0 :
					if retval != 0:
						raise ProcessRunningError('pid(%d) exit (%d)'%(removepid,retval))
				time.sleep(1.0)
				sys.stdout.write('P%d'%(len(streampids)))
				sys.stdout.flush()
			# for sleep to make things flush into the buffer
			self.__check_leakout()
		finally:
			self.__clean_leakout()
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

def Runtest(cfname,variables=[]):
	utcfg = xunit.config.XUnitConfig()
	SetOuterVariables(utcfg,variables)
	# now to add the %(build.topdir)s
	_appath=os.path.abspath(os.path.join(os.path.abspath(os.path.dirname(__file__)),'..','..','..','..'))
	utcfg.SetValue('build','topdir',_appath)
	utcfg.LoadFile(cfname)
	# we need to set the variable to be overwrited
	SetOuterVariables(utcfg,variables)

	# now we should get
	units = utcfg.GetUnitTests()
	suites = xunit.suite.XUnitSuiteBase()
	if len(units) > 0:
		for u in units:
			suites.LoadCase(u)

	# now we should set for the case verbose is none we debug our self information
	_res = xunit.result.XUnitResultBase()
	for s in suites:
		s(_res)
		if _res.shouldStop:
			break

	_ret = -1
	if _res.Fails() == 0 and _res.UnexpectFails() ==0 and _res.UnexpectSuccs() == 0:
		_ret = 0

	_res.ResultAll()

	return _ret



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


if __name__ == '__main__':
	args = OptionParser()
	args.add_option('-v','--verbose',action='count',dest='verbose',help='verbose mode if -vvv debug most')
	args.add_option('-f','--failfast',action="store_true",dest="failfast",help="failfast mode")
	args.add_option('-x','--xmllog',action='store',dest='xmllog',nargs=1,help='set xmllog file defautlt is none')
	args.add_option('-D','--variable',action="callback",callback=Parse_Callback,type="string",nargs=1,help='specify a variable format is [section].option=value')
	args.add_option('-V','--debug',action="callback",callback=Parse_Callback,help='debug mode ')

	options ,nargs = args.parse_args(sys.argv[1:])
	if len (nargs) < 1:
		Usage(args,3,"need one config files")

	_vars = []
	utcfg = xunit.config.XUnitConfig()

	
	if options.verbose:
		logging.basicConfig(level=logging.INFO,format="%(levelname)-8s %(asctime)s[%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")
		_vars.append('[global].debug.mode=y')
		if options.verbose >= 3:
			_vars.append('[.__main__].xmllevel=5')

	if options.failfast:
		_vars.append('[global].failfast=y')
	if options.xmllog:
		_vars.append('[global].xmllog=%s'%(options.xmllog))

	#logging.info('opt %r'%(options))
	if hasattr(options,'variables'):
		_vars.extend(options.variables)
	_ret = Runtest(nargs[0],_vars)
	if _ret != 0:
		sys.exit(3)
	sys.exit(0)
