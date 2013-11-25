#! python

import socket
import os
import sys
import xunit.utils.exception
import struct
import logging
import subprocess
import re
class RealStreamConnectError(xunit.utils.exception.XUnitException):
	pass
class RealStreamInvalidError(xunit.utils.exception.XUnitException):
	pass
class RealStreamFormatError(xunit.utils.exception.XUnitException):
	pass
class RealStreamReceiveError(xunit.utils.exception.XUnitException):
	pass
class RealStreamUnsupportedSystem(xunit.utils.exception.XUnitException):
	pass

class RealStreamSetRMemMaxError(xunit.utils.exception.XUnitException):
	pass
class RealStreamSystemCmdError(xunit.utils.exception.XUnitException):
	pass

class RealStream:
	def __init__(self,hostport,timeout=3):
		self.__sock=None
		sarr = hostport.split(':')
		if len(sarr) <=1:
			raise RealStreamInvalidError('not valid (%s)'%(hostport))
		self.__host= sarr[0]
		try:
			self.__port = int(sarr[1])
		except:
			raise RealStreamInvalidError('not valid (%s)'%(hostport))

	def __RecvLength(self,length):
		hasrecv = 0
		buf = ''
		while hasrecv < length:
			curlen = length - hasrecv
			curbuf = self.__sock.recv(curlen)
			if curbuf is None or len(curbuf) == 0:
				raise RealStreamReceiveError('can not receive %d'%(length))
			buf += curbuf
			hasrecv = len(buf)
		return buf
	def __IsLinuxSystem(self):
		vpat = re.compile('linux',re.I)
		if vpat.search(sys.platform):
			return 1
		else:
			return 0

	def __ReadRMemMax(self):
		cmd = 'cat /proc/sys/net/core/rmem_max'
		sp = subprocess.Popen(cmd,shell=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
		op = sp.stdout
		ls = op.readlines()
		l = ls[0]
		l = l.rstrip('\r\n')
		return int(l)

	def __SetRMemMax(self,bufdsize):
		cmd = 'sudo su -c \'echo %d >/proc/sys/net/core/rmem_max\''%(bufdsize)
		ret = os.system(cmd)
		if ret != 0:
			raise RealStreamSystemCmdError('can not run cmd(%s) succ please to set sudo no password running'%(cmd))
		return
	def __SetRcvBuf(self,bufsize):
		# now to set the socket buffer size
		osize = self.__sock.getsockopt(socket.SOL_SOCKET,socket.SO_RCVBUF)
		if (bufsize << 1) > osize:
			# now we set the buffer
			self.__sock.setsockopt(socket.SOL_SOCKET,socket.SO_RCVBUF,bufsize)
			nsize = self.__sock.getsockopt(socket.SOL_SOCKET,socket.SO_RCVBUF)
			if nsize < (bufsize * 2):
				if not self.__IsLinuxSystem():
					raise RealStreamUnsupportedSystem('can not reset the rmem_max in platform %s'%(sys.platform))
				self.__SetRMemMax((bufsize << 1))
				self.__sock.setsockopt(socket.SOL_SOCKET,socket.SO_RCVBUF,bufsize)
				nsize = self.__sock.getsockopt(socket.SOL_SOCKET,socket.SO_RCVBUF)
				if nsize < (bufsize << 1):
					raise RealStreamSetRMemMaxError('can not expand buffer size   %d'%(bufsize))
		return
		


	def Connect(self):
		try:
			self.CloseSocket()
			self.__sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
			#self.__SetRcvBuf((1<<23))
			self.__sock.connect((self.__host,self.__port))			
		except:
			raise RealStreamConnectError('connect (%s:%d) error'%(self.__host,self.__port))

	def Establish(self,streamid=0):
		try:
			if self.__sock is None:
				raise RealStreamConnectError('Not connected')
			buf = struct.pack('<I',streamid)
			self.__sock.send(buf)
			buf = self.__RecvLength(6)
			#logging.info('received %s',repr(buf))
			streamtype ,width,height = struct.unpack('<HHH',buf)
			return streamtype,width,height
		except :
			raise RealStreamFormatError('can not get format (%d)'%(streamid))
		return None,None,None

	def GetFrameMedia(self):
		try:
			buf = self.__RecvLength(4)
			length = struct.unpack('<I',buf)
			#logging.info('length %d 0x%x'%(length[0],length[0]));
			return self.__RecvLength(length[0])
		except:
			raise RealStreamReceiveError('can not receive media')
	def CloseSocket(self):
		if self.__sock:			
			self.__sock.close()
		self.__sock = None
	def __del__(self):
		self.CloseSocket()
		self.__host = None
		self.__port = None
