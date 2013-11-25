#ifndef __GMI_XML_API_H__
#define __GMI_XML_API_H__

#include <stdio.h>
#include "gmi_system_headers.h"


#define GMI_CONFIG_READ_ONLY  0
#define GMI_CONFIG_READ_WRITE  1
#define MAX_CHAR_BUF_LEN  512

/*===============================================================
func name:GMI_XmlOpen
func:Open xml file ,retrun file Handle
input: FileName :Open file name.
         Handle : file hander.
return:success--return file handler, failed -- return NULL
--------------------------------------------------------------------*/
GMI_RESULT GMI_XmlOpen(const char_t *FileName, FD_HANDLE  *Handle);

/*===============================================================
func name:GMI_XmlRead
func:Read xml node content
input: Handle : file handler.  
         Path : node path for example ,/a/b/c/ . 
         keyWord: node name
         Default:if node non-existent,set Default value
         Context: return node value.
return:success--return node value, failed -- return error code
--------------------------------------------------------------------*/
GMI_RESULT GMI_XmlRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord,const  float_t Default , float_t *Context, int32_t flags);
GMI_RESULT GMI_XmlRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord,const  int32_t Default , int32_t *Context, int32_t flags);
GMI_RESULT GMI_XmlRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const uint16_t Default , uint16_t *Context, int32_t flags);
GMI_RESULT GMI_XmlRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const int16_t Default , int16_t *Context, int32_t flags);
GMI_RESULT GMI_XmlRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const uint8_t Default , uint8_t *Context, int32_t flags);
GMI_RESULT GMI_XmlRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const int8_t Default , int8_t *Context, int32_t flags);
GMI_RESULT GMI_XmlRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord,const  char_t *Default, char_t *Context, int32_t flags);

/*===============================================================
func name:GMI_XmlWrite
func:Write xml node content,
input:Handle :  file handler.  
         Path :   node path for example ,/a/b/c/ . 
         keyWord:  node name
         Key:  Set node value
return:success--return SUCCESS, failed -- return ERROR CODE
--------------------------------------------------------------------*/
GMI_RESULT GMI_XmlWrite(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const float_t Key);
GMI_RESULT GMI_XmlWrite(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const int32_t Key);
GMI_RESULT GMI_XmlWrite(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const uint16_t Key);
GMI_RESULT GMI_XmlWrite(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const int16_t Key);
GMI_RESULT GMI_XmlWrite(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const uint8_t Key);
GMI_RESULT GMI_XmlWrite(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const int8_t Key);
GMI_RESULT GMI_XmlWrite(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const char_t *Key);

/*===============================================================
func name:GMI_XmlFileSave
func: Save Xml  file 
input: FileName :Close the file name.
         Handle : file hander.
return:success--return Success, failed -- return Error Code
--------------------------------------------------------------------*/
GMI_RESULT GMI_XmlFileSave(FD_HANDLE Handle);

GMI_RESULT GMI_XmlFileFailSave(FD_HANDLE Handle);


#endif


