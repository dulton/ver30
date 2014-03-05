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
import xunit.extlib.sdkproto.userinfo
import hashlib

def OptsHasName(opts,name):
	if not hasattr(opts,name):
		return 0
	if getattr(opts,name) is None:
		return 0

	return 1


def __GetUserInfo(ssock):
	userinfos = ssock.GetUserInfo()
	userinfo = None
	print('==================================')
	for ui in userinfos:
		print('%s'%(repr(ui)))

	print('==================================')
	return userinfos
	
def GetUserInfoUserPassword(host,port,username,password):
	ssock = xunit.extlib.sdksock.SdkUserInfoSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	return __GetUserInfo(ssock)


def __GetPassKey(hwaddr):
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

def __SetModifyUsers(userinfos,passkey ,opts,delone=0):
	newusers = []
	if delone:
		totalusers = len(opts.userdelete)
	else:
		totalusers = len(opts.username)
	i = 0
	while i < totalusers:
		if delone == 1:
			cname = opts.userdelete[i]
		else:
			cname = opts.username[i]
		cuser = None
		for ui in userinfos:
			if ui.UserName() == cname:
				cuser = ui
				break
		if cuser is None:
			if delone == 1:
				raise Exception('could not find user %s'%(opts.userdelete[i]))
			cuser = xunit.extlib.sdkproto.userinfo.UserInfo(passkey)
		if delone :
			cuser.UserName(opts.userdelete[i])
		else:
			cuser.UserName(opts.username[i])
			cuser.UserPass(opts.userpass[i])
			cuser.UserFlag(opts.userflag[i])
			cuser.UserLevel(opts.userlevel[i])
		newusers.append(cuser)
		i += 1
	return newusers
	

def __SetUserInfo(ssock,opts):
	userinfos = __GetUserInfo(ssock)
	userinfo = None
	netinfos = ssock.GetIpInfo()
	hwaddr = netinfos[0].HwAddr()
	passkey = __GetPassKey(hwaddr)

	newusers = __SetModifyUsers(userinfos,passkey,opts)

	logging.info('\n')
	ssock.SetUserInfo(newusers)
	logging.info('\n')

	__GetUserInfo(ssock)
	return

def SetUserInfoUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkUserInfoSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('session id %s'%(sesid))
	return __SetUserInfo(ssock,opts)

def __DelUserInfo(ssock,opts):
	userinfos = __GetUserInfo(ssock)
	netinfos = ssock.GetIpInfo()
	hwaddr = netinfos[0].HwAddr()
	passkey = __GetPassKey(hwaddr)
	userinfo = xunit.extlib.sdkproto.userinfo.UserInfo(passkey)
	newusers = __SetModifyUsers(userinfos,passkey,opts,1)
	ssock.DelUserInfo(newusers)
	__GetUserInfo(ssock)
	return 

def DelUserInfoPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkUserInfoSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	return __DelUserInfo(ssock,opts)



def Usage(opt,ec,msg=None):
	fp = sys.stderr
	if ec == 0:
		fp = sys.stdout
	if msg :
		fp.write('%s\n'%(msg))
	opt.print_help(fp)
	sys.exit(ec)

def Md5(s):
	m = hashlib.md5()
	m.update(s)
	return m.hexdigest()



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

	if OptsHasName(opts,'userdelete'):
		return

	if OptsHasName(opts,'username') and OptsHasName(opts,'userpass') and OptsHasName(opts,'userflag') and OptsHasName(opts,'userlevel'):
		if len(opts.username) == len(opts.userpass) and len(opts.userpass) == len(opts.userflag) and len(opts.userlevel) == len(opts.userflag):
			return
		


	Usage(args,3,'please specify --getcfg or --username  and --userpass and --userflag and --userlevel')

	return 


def SetArgsInt(args,name):
	args.add_option('--%s'%(name),action='store',nargs=1,type='int',dest='%s'%(name),default=None,help='to set %s'%(name))
	return

def SetArgsStr(args,name):
	args.add_option('--%s'%(name),action='store',nargs=1,type='string',dest='%s'%(name),default=None,help='to set %s'%(name))
	return

def SetArgsAppendStr(args,name):
	args.add_option('--%s'%(name),action='append',nargs=1,type='string',dest='%s'%(name),default=None,help='to append %s'%(name))
	return

def SetArgsAppendInt(args,name):
	args.add_option('--%s'%(name),action='append',nargs=1,type='int',dest='%s'%(name),default=None,help='to append %s'%(name))
	return

def main():
	args = OptionParser()
	args.add_option('-v','--verbose',action='store_true',dest='verbose',help='verbose mode')
	args.add_option('-H','--host',action='store',dest='host',nargs=1,help='specify the host')
	args.add_option('-p','--port',action='store',type='int',dest='port',nargs=1,help='specify the port default is 23')
	args.add_option('-u','--user',action='store',dest='user',nargs=1,help='specify username')
	args.add_option('-P','--pass',action='store',dest='password',nargs=1,help='specify password')
	args.add_option('-s','--session',action='store',dest='session',type='int',nargs=1,help='specify sessionid if has this ,would not login with user password')
	args.add_option('-g','--getcfg',action='store_true',dest='getcfg',default=None,help='to get userinfo')
	SetArgsAppendStr(args,'username')
	SetArgsAppendStr(args,'userpass')
	SetArgsAppendInt(args,'userflag')
	SetArgsAppendInt(args,'userlevel')
	SetArgsAppendStr(args,'userdelete')
	
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)

	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(asctime)s %(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	if options.session :
		pass
	else:
		if options.getcfg:
			GetUserInfoUserPassword(options.host,options.port,options.user,options.password)
		elif options.userdelete:
			DelUserInfoPassword(options.host,options.port,options.user,options.password,options)
		else:
			SetUserInfoUserPassword(options.host,options.port,options.user,options.password,options)
	return

if __name__ == '__main__':
	main()
