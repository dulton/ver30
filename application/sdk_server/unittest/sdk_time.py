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
	if hasattr(opts,name) and getattr(opts,name) is not None:
		return 1
	return 0

def SetTimeUserPassword(host,port,username,password,opts):
	ssock = xunit.extlib.sdksock.SdkTimeSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('session id %s'%(sesid))
	timetype,systime,ntpserver,timezone = ssock.GetTimeCfg()
	
	print('before set\n%s%s%s%s'%(str(timetype),str(systime),str(ntpserver),str(timezone)))
	sntpserver = None
	stimetype = None
	ssystime = None
	stimezone = None
	if OptsHasName(opts,'ntpserverntpaddr1') :
		if sntpserver is None:
			sntpserver = ntpserver
		sntpserver.NtpAddr1(opts.ntpserverntpaddr1)
	
	if OptsHasName(opts,'ntpserverntpaddr2') :
		if sntpserver is None:
			sntpserver = ntpserver
		sntpserver.NtpAddr2(opts.ntpserverntpaddr2)
	if OptsHasName(opts,'ntpserverntpaddr3') :
		if sntpserver is None:
			sntpserver = ntpserver
		sntpserver.NtpAddr3(opts.ntpserverntpaddr3)

	if OptsHasName(opts,'systimeyear'):
		if ssystime is None:
			ssystime = systime
		ssystime.Year(opts.systimeyear)
	
	if OptsHasName(opts,'systimemonth'):
		if ssystime is None:
			ssystime = systime
		ssystime.Month(opts.systimemonth)
		
	if OptsHasName(opts,'systimeday'):
		if ssystime is None:
			ssystime = systime
		ssystime.Day(opts.systimeday)
	
	if OptsHasName(opts,'systimehour'):
		if ssystime is None:
			ssystime = systime
		ssystime.Hour(opts.systimehour)

	if OptsHasName(opts,'systimeminute'):
		if ssystime is None:
			ssystime = systime
		ssystime.Minute(opts.systimeminute)

	if OptsHasName(opts,'systimesecond'):
		if ssystime is None:
			ssystime = systime
		ssystime.Second(opts.systimesecond)

	

	ssock.SetTimeCfg(stimetype,ssystime,sntpserver,stimezone)
	timetype,systime,ntpserver,timezone = ssock.GetTimeCfg()
	print('after set\n%s%s%s%s'%(str(timetype),str(systime),str(ntpserver),str(timezone)))


	return
	
def GetTimeUserPassword(host,port,username,password):
	ssock = xunit.extlib.sdksock.SdkTimeSock(host,port)
	sesid = ssock.LoginUserPass(username,password)
	logging.info('session id %s'%(sesid))
	timetype,systime,ntpserver,timezone = ssock.GetTimeCfg()
	print('%s%s%s%s'%(str(timetype),str(systime),str(ntpserver),str(timezone)))
	return


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

	if 	OptsHasName(opts,'timezonetimezone') or OptsHasName(opts,'timezonetimezonename') or \
		OptsHasName(opts,'ntpserverntpaddr1') or OptsHasName(opts,'ntpserverntpaddr2') or \
		OptsHasName(opts,'ntpserverntpaddr3') or OptsHasName(opts,'systimeyear') or \
		OptsHasName(opts,'systimemonth') or OptsHasName(opts,'systimeday') or \
		OptsHasName(opts,'systimehour') or OptsHasName(opts,'systimeminute') or \
		OptsHasName(opts,'systimesecond') or OptsHasName(args,'timetypetype') or \
		OptsHasName(args,'timetypentpinterval'):
		return 

	Usage(args,3,'please specify one name')
	return 


def SetArgsInt(args,name):
	args.add_option('--%s'%(name),action='store',nargs=1,type='int',dest='%s'%(name),default=None,help='to set %s'%(name))
	return
	
def SetArgsString(args,name):
	args.add_option('--%s'%(name),action='store',nargs=1,type='string',dest='%s'%(name),default=None,help='set %s'%(name))
	return

def main():
	args = OptionParser()
	args.add_option('-v','--verbose',action='store_true',dest='verbose',help='verbose mode')
	args.add_option('-H','--host',action='store',dest='host',nargs=1,help='specify the host')
	args.add_option('-p','--port',action='store',type='int',dest='port',nargs=1,help='specify the port default is 23')
	args.add_option('-u','--user',action='store',dest='user',nargs=1,help='specify username')
	args.add_option('-P','--pass',action='store',dest='password',nargs=1,help='specify password')
	args.add_option('-s','--session',action='store',dest='session',type='int',nargs=1,help='specify sessionid if has this ,would not login with user password')
	args.add_option('-g','--getcfg',action='store_true',dest='getcfg',default=None,help='to get showcfg')

	SetArgsInt(args,'timezonetimezone')
	SetArgsString(args,'timezonetimezonename')
	SetArgsString(args,'ntpserverntpaddr1')
	SetArgsString(args,'ntpserverntpaddr2')
	SetArgsString(args,'ntpserverntpaddr3')
	SetArgsInt(args,'systimeyear')
	SetArgsInt(args,'systimemonth')
	SetArgsInt(args,'systimeday')
	SetArgsInt(args,'systimehour')
	SetArgsInt(args,'systimeminute')
	SetArgsInt(args,'systimesecond')
	
	SetArgsInt(args,'timetypetype')
	SetArgsInt(args,'timetypentpinterval')
	
	options ,nargs = args.parse_args(sys.argv[1:])

	VerifyOptions(options,args)

	if options.verbose :
		logging.basicConfig(level=logging.INFO,format="%(levelname)-8s [%(filename)-10s:%(funcName)-20s:%(lineno)-5s] %(message)s")

	if options.session :
		pass
	else:
		if options.getcfg:
			GetTimeUserPassword(options.host,options.port,options.user,options.password)
		else:		
			SetTimeUserPassword(options.host,options.port,options.user,options.password,options)
	return

if __name__ == '__main__':
	main()

