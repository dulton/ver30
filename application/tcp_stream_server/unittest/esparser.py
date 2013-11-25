#! python


import xunit.utils.exception
import struct
import logging

class ESParserTooShortError(xunit.utils.exception.XUnitException):
	pass
class ESParserInvalidFormatError(xunit.utils.exception.XUnitException):
	pass

def IsIFrame(buf):
	if len(buf) < 6:
		raise ESParserTooShortError('buf len too short %d'%(len(buf)))
	if ord(buf[0]) != 0 or ord(buf[1]) != 0 or ord(buf[2]) != 0x0:
		return 0
	if ord(buf[3]) != 0x1 or ord(buf[4]) != 0x9 or ord(buf[5]) != 0x10:
		return 0
	return 1


def IsPFrame(buf):
	if len(buf) < 6:
		raise ESParserTooShortError('buf len too short %d'%(len(buf)))
	if ord(buf[0]) != 0 or ord(buf[1]) != 0 or ord(buf[2]) != 0x0:
		return 0
	if ord(buf[3]) != 0x1 or ord(buf[4]) != 0x9 or ord(buf[5]) != 0x30:
		return 0
	return 1

def GetFrameNum(buf):
	if IsIFrame(buf):
		return 0
	if not IsPFrame(buf):
		raise ESParserInvalidFormatError('not valid frame %s'%(repr(buf[:6])))
	if len(buf) < 32:
		raise ESParserTooShortError('buf len too short %d'%(len(buf)))
	num = ord(buf[30])
	verinum = ord(buf[31])
	if num != verinum :
		raise ESParserInvalidFormatError('not valid frame num %d != %d'%(num,verinum))
	return num


