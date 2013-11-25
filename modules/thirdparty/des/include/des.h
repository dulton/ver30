#ifndef __DES_H__
#define __DES_H__

#ifdef __cplusplus
extern "C" {
#endif 

//password length
#define LEN_DES_PASSWORD 32

/*===============================================================
func name:DES_Encrypt
func:encrypt data by DES
input:plainText--original text;
        plainTextLen -- length of original text;
        keyStr--key of des
output:cipherText--result of encryption by des
            cipherTextLen--length of result of encryption
return:success--return 0, 
	failed -- return -1
---------------------------------------------------------------------*/	
int DES_Encrypt(char *plainText, int plainTextLen, char *keyStr,char *cipherText, int *cipherTextLen);


/*===============================================================
func name:DES_Decrypt
func:decrypt data by DES
input:cipherText--encryped text;
        cipherTextLen--length of encryped text;
        keyStr--key of des
output:plainText--original text;
            plainTextLen -- length of original text;
return:success--return 0, 
	failed -- return -1
---------------------------------------------------------------------*/	
int DES_Decrypt(char *cipherText, int cipherTextLen, char *keyStr, char *plainText, int *plainTextLen);

#ifdef __cplusplus
}
#endif 


#endif
